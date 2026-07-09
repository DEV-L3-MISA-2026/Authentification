using Microsoft.AspNetCore.Mvc;
using SourceAFIS;
using sourceAFIS_mvc_test.Repositories;
using sourceAFIS_mvc_test.Services;
using System.Collections.Concurrent;

namespace sourceAFIS_mvc_test.Controllers;

[ApiController]
[Route("api/mobile-session")]
public class MobileSessionController : ControllerBase
{
    private readonly ITemplateExtractorService _templateExtractor;
    private readonly ITemplateRepository _templateRepository;
    private static readonly ConcurrentDictionary<string, MobileSession> Sessions = new();
    private static PendingSessionInfo? _pendingSession;
    private static readonly object _pendingLock = new();
    private const double Threshold = 40;

    public MobileSessionController(
        ITemplateExtractorService templateExtractor,
        ITemplateRepository templateRepository)
    {
        _templateExtractor = templateExtractor;
        _templateRepository = templateRepository;
    }

    /// <summary>
    /// Endpoint pollé par la page mobile (index.html) - retourne la session en attente
    /// </summary>
    [HttpGet("pending")]
    public ActionResult<PendingResponse> GetPending()
    {
        lock (_pendingLock)
        {
            var pending = _pendingSession;
            if (pending != null)
            {
                _pendingSession = null; // Consomme la demande
                return Ok(new PendingResponse
                {
                    SessionId = pending.SessionId,
                    Mode = pending.Mode,
                    UserId = pending.UserId,
                    TemplateId = pending.TemplateId
                });
            }
        }
        return Ok(new PendingResponse { SessionId = null });
    }

    /// <summary>
    /// Initialise une session de capture mobile
    /// </summary>
    [HttpPost("init")]
    public ActionResult<InitResponse> Init([FromBody] InitRequest request)
    {
        var sessionId = Guid.NewGuid().ToString("N")[..12];
        var session = new MobileSession
        {
            SessionId = sessionId,
            Mode = request.Mode,
            UserId = request.UserId,
            TemplateId = request.TemplateId,
            CreatedAt = DateTime.UtcNow,
            Status = "waiting"
        };

        Sessions[sessionId] = session;

        // Signaler au téléphone qu'une session est prête
        lock (_pendingLock)
        {
            _pendingSession = new PendingSessionInfo
            {
                SessionId = sessionId,
                Mode = request.Mode,
                UserId = request.UserId,
                TemplateId = request.TemplateId
            };
        }

        // Nettoyage des sessions expirées (plus de 5 minutes)
        CleanExpiredSessions();

        return Ok(new InitResponse
        {
            SessionId = sessionId,
            Status = "waiting"
        });
    }

    /// <summary>
    /// Le téléphone soumet l'image capturée
    /// </summary>
    [HttpPost("submit")]
    public async Task<ActionResult<SubmitResponse>> Submit([FromBody] SubmitRequest request)
    {
        if (!Sessions.TryGetValue(request.SessionId, out var session))
        {
            return NotFound(new SubmitResponse
            {
                Success = false,
                Message = "Session introuvable ou expirée"
            });
        }

        if (session.Status != "waiting")
        {
            return BadRequest(new SubmitResponse
            {
                Success = false,
                Message = $"Session déjà traitée (statut: {session.Status})"
            });
        }

        try
        {
            var base64 = request.ImageData.Contains(',')
                ? request.ImageData.Split(',')[1]
                : request.ImageData;

            var imageBytes = Convert.FromBase64String(base64);

            if (session.Mode == "enroll")
            {
                // Créer le template et l'enregistrer en DB
                var templateBytes = await _templateExtractor.CreateTemplateAsync(imageBytes);
                var template = new Models.Template
                {
                    TemplateData = templateBytes,
                    UserId = session.UserId > 0 ? session.UserId : 1,
                    CreatedAt = DateTime.UtcNow
                };

                var created = await _templateRepository.CreateAsync(template);

                session.Status = "completed";
                session.Result = new SessionResult
                {
                    Success = true,
                    TemplateId = created.Id,
                    Template = Convert.ToBase64String(templateBytes),
                    Message = "Empreinte enregistrée avec succès"
                };

                return Ok(new SubmitResponse
                {
                    Success = true,
                    Message = "Empreinte enregistrée avec succès",
                    TemplateId = created.Id
                });
            }
            else if (session.Mode == "verify")
            {
                if (session.TemplateId <= 0)
                {
                    session.Status = "error";
                    session.Result = new SessionResult
                    {
                        Success = false,
                        Message = "TemplateId requis pour la vérification"
                    };

                    return BadRequest(new SubmitResponse
                    {
                        Success = false,
                        Message = "TemplateId requis pour la vérification"
                    });
                }

                // Récupérer le template stocké
                var storedTemplate = await _templateRepository.GetByIdAsync(session.TemplateId);
                if (storedTemplate?.TemplateData == null)
                {
                    session.Status = "error";
                    session.Result = new SessionResult
                    {
                        Success = false,
                        Message = "Template non trouvé en base de données"
                    };

                    return NotFound(new SubmitResponse
                    {
                        Success = false,
                        Message = "Template non trouvé en base de données"
                    });
                }

                // Comparer les empreintes
                var probeTemplateBytes = await _templateExtractor.CreateTemplateAsync(imageBytes);
                var probeTemplate = new FingerprintTemplate(probeTemplateBytes);
                var candidateTemplate = new FingerprintTemplate(storedTemplate.TemplateData);
                double score = new FingerprintMatcher(probeTemplate).Match(candidateTemplate);

                bool verified = score >= Threshold;
                int scoreInt = (int)Math.Round(score);

                session.Status = "completed";
                session.Result = new SessionResult
                {
                    Success = verified,
                    Score = scoreInt,
                    Message = verified ? "Empreinte vérifiée" : "Empreinte non reconnue"
                };

                return Ok(new SubmitResponse
                {
                    Success = verified,
                    Message = verified ? "Empreinte vérifiée" : "Empreinte non reconnue",
                    Score = scoreInt
                });
            }
            else
            {
                session.Status = "error";
                session.Result = new SessionResult
                {
                    Success = false,
                    Message = $"Mode inconnu: {session.Mode}"
                };

                return BadRequest(new SubmitResponse
                {
                    Success = false,
                    Message = $"Mode inconnu: {session.Mode}"
                });
            }
        }
        catch (Exception ex)
        {
            session.Status = "error";
            session.Result = new SessionResult
            {
                Success = false,
                Message = $"Erreur: {ex.Message}"
            };

            return StatusCode(500, new SubmitResponse
            {
                Success = false,
                Message = $"Erreur: {ex.Message}"
            });
        }
    }

    /// <summary>
    /// Le PC récupère le résultat de la session (polling)
    /// </summary>
    [HttpGet("{sessionId}/result")]
    public ActionResult<SessionResult> GetResult(string sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var session))
        {
            return NotFound(new SessionResult
            {
                Success = false,
                Message = "Session introuvable ou expirée"
            });
        }

        if (session.Status == "waiting")
        {
            return Ok(new SessionResult
            {
                Success = false,
                Status = "waiting",
                Message = "En attente de capture..."
            });
        }

        // Nettoyer la session après récupération du résultat
        Sessions.TryRemove(sessionId, out _);

        return Ok(session.Result ?? new SessionResult
        {
            Success = false,
            Message = "Erreur inconnue"
        });
    }

    private void CleanExpiredSessions()
    {
        var cutoff = DateTime.UtcNow.AddMinutes(-5);
        var expired = Sessions.Where(s => s.Value.CreatedAt < cutoff).Select(s => s.Key).ToList();
        foreach (var key in expired)
        {
            Sessions.TryRemove(key, out _);
        }
    }

    // --- Modèles ---

    public record InitRequest
    {
        public string Mode { get; init; } = "enroll"; // "enroll" ou "verify"
        public int UserId { get; init; }
        public int TemplateId { get; init; }
    }

    public record InitResponse
    {
        public string SessionId { get; init; } = string.Empty;
        public string Status { get; init; } = string.Empty;
    }

    public record SubmitRequest
    {
        public string SessionId { get; init; } = string.Empty;
        public string ImageData { get; init; } = string.Empty;
    }

    public record SubmitResponse
    {
        public bool Success { get; init; }
        public string Message { get; init; } = string.Empty;
        public int TemplateId { get; init; }
        public int Score { get; init; }
    }

    public class MobileSession
    {
        public string SessionId { get; set; } = string.Empty;
        public string Mode { get; set; } = "enroll";
        public int UserId { get; set; }
        public int TemplateId { get; set; }
        public DateTime CreatedAt { get; set; }
        public string Status { get; set; } = "waiting"; // waiting, completed, error
        public SessionResult? Result { get; set; }
    }

    public class SessionResult
    {
        public bool Success { get; init; }
        public string Status { get; init; } = "completed";
        public string Message { get; init; } = string.Empty;
        public int TemplateId { get; init; }
        public string Template { get; init; } = string.Empty;
        public int Score { get; init; }
    }

    public class PendingSessionInfo
    {
        public string SessionId { get; set; } = string.Empty;
        public string Mode { get; set; } = "enroll";
        public int UserId { get; set; }
        public int TemplateId { get; set; }
    }

    public record PendingResponse
    {
        public string? SessionId { get; init; }
        public string Mode { get; init; } = "enroll";
        public int UserId { get; init; }
        public int TemplateId { get; init; }
    }
}
