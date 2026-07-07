using Microsoft.AspNetCore.Mvc;
using SourceAFIS;
using sourceAFIS_mvc_test.Models;
using sourceAFIS_mvc_test.Repositories;
using sourceAFIS_mvc_test.Services;

namespace sourceAFIS_mvc_test.Controllers;

/// <summary>
/// API endpoints pour Blazor WebAssembly - persistance DB des templates avec userId + templateId
/// </summary>
[ApiController]
[Route("wasm/fingerprint")]
public class FingerprintWasmController : ControllerBase
{
    private readonly ITemplateExtractorService _templateExtractor;
    private readonly ITemplateRepository _templateRepository;
    private const double Threshold = 40;

    public FingerprintWasmController(
        ITemplateExtractorService templateExtractor,
        ITemplateRepository templateRepository)
    {
        _templateExtractor = templateExtractor;
        _templateRepository = templateRepository;
    }

    /// <summary>
    /// Enrollment - crée un template SourceAFIS à partir d'une image base64
    /// Stocke le template en DB et le retourne
    /// Retourne: templateId, template (base64)
    /// </summary>
    [HttpPost("enroll")]
    public async Task<ActionResult<WasmEnrollResponse>> Enroll([FromBody] WasmEnrollRequest request)
    {
        try
        {
            if (request == null || string.IsNullOrWhiteSpace(request.ImageData))
            {
                return BadRequest(new WasmEnrollResponse
                {
                    Success = false,
                    Message = "ImageData requise"
                });
            }

            var base64 = request.ImageData.Contains(',') 
                ? request.ImageData.Split(',')[1] 
                : request.ImageData;
            
            var imageBytes = Convert.FromBase64String(base64);
            var templateBytes = await _templateExtractor.CreateTemplateAsync(imageBytes);

            var template = new Template
            {
                TemplateData = templateBytes,
                UserId = request.UserId > 0 ? request.UserId : 1,
                CreatedAt = DateTime.UtcNow
            };

            var created = await _templateRepository.CreateAsync(template);

            return Ok(new WasmEnrollResponse
            {
                Success = true,
                Message = "Template créé avec succès",
                TemplateId = created.Id,
                Template = Convert.ToBase64String(templateBytes)
            });
        }
        catch (Exception ex)
        {
            return StatusCode(500, new WasmEnrollResponse
            {
                Success = false,
                Message = $"Erreur: {ex.Message}"
            });
        }
    }

    /// <summary>
    /// Vérification - compare une image avec un template stocké en DB
    /// Entrée: userId, templateId, ImageData
    /// Retourne: success, score, template
    /// </summary>
    [HttpPost("verify")]
    public async Task<ActionResult<WasmVerifyResponse>> Verify([FromBody] WasmVerifyRequest request)
    {
        try
        {
            if (request == null || string.IsNullOrWhiteSpace(request.ImageData))
            {
                return BadRequest(new WasmVerifyResponse
                {
                    Success = false,
                    Message = "ImageData requise"
                });
            }

            if (request.TemplateId <= 0)
            {
                return BadRequest(new WasmVerifyResponse
                {
                    Success = false,
                    Message = "TemplateId requis"
                });
            }

            // Récupérer le template stocké depuis la DB
            var storedTemplate = await _templateRepository.GetByIdAsync(request.TemplateId);
            if (storedTemplate == null || storedTemplate.TemplateData == null)
            {
                return NotFound(new WasmVerifyResponse
                {
                    Success = false,
                    Message = "Template non trouvé"
                });
            }

            var probeBase64 = request.ImageData.Contains(',') 
                ? request.ImageData.Split(',')[1] 
                : request.ImageData;
            
            var probeImageBytes = Convert.FromBase64String(probeBase64);
            var probeTemplateBytes = await _templateExtractor.CreateTemplateAsync(probeImageBytes);

            var probeTemplate = new FingerprintTemplate(probeTemplateBytes);
            var candidateTemplate = new FingerprintTemplate(storedTemplate.TemplateData);
            double score = new FingerprintMatcher(probeTemplate).Match(candidateTemplate);
            
            bool verified = score >= Threshold;
            int scoreInt = (int)Math.Round(score);

            return Ok(new WasmVerifyResponse
            {
                Success = verified,
                Score = scoreInt,
                Message = verified ? "Empreinte vérifiée" : "Empreinte non reconnue",
                Template = Convert.ToBase64String(probeTemplateBytes)
            });
        }
        catch (Exception ex)
        {
            return StatusCode(500, new WasmVerifyResponse
            {
                Success = false,
                Message = $"Erreur: {ex.Message}"
            });
        }
    }

    public record WasmEnrollRequest
    {
        public string ImageData { get; init; } = string.Empty;
        public int UserId { get; init; }
    }

    public record WasmEnrollResponse
    {
        public bool Success { get; init; }
        public string Message { get; init; } = string.Empty;
        public int TemplateId { get; init; }
        public string Template { get; init; } = string.Empty;
    }

    public record WasmVerifyRequest
    {
        public string ImageData { get; init; } = string.Empty;
        public int UserId { get; init; }
        public int TemplateId { get; init; }
    }

    public record WasmVerifyResponse
    {
        public bool Success { get; init; }
        public string Message { get; init; } = string.Empty;
        public int Score { get; init; }
        public string Template { get; init; } = string.Empty;
    }
}