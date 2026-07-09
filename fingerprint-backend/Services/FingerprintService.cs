using SourceAFIS;
using sourceAFIS_mvc_test.Models;
using sourceAFIS_mvc_test.Repositories;

namespace sourceAFIS_mvc_test.Services;

public class FingerprintService : IFingerprintService
{
    private readonly ITemplateRepository _templateRepository;
    private readonly IAuthLogRepository _authLogRepository;

    private const double Threshold = 40;

    public FingerprintService(ITemplateRepository templateRepository,IAuthLogRepository authLogRepository)
    {
        _templateRepository = templateRepository;
        _authLogRepository = authLogRepository;
    }

    public async Task<(bool Success, int Score)> VerifyAsync(byte[] probeTemplateBytes, int userId)
    {
        Console.WriteLine($"[Service] Verify démarré -> userId={userId}");

        var templates = await _templateRepository
            .GetByUserIdAsync(userId);

        Console.WriteLine($"[Service] Templates trouvés -> count={templates.Count()}");

        if (!templates.Any())
        {
            await LogAttempt(userId, 0, false);
            Console.WriteLine("[Service] Verify échoué : aucun template");
            return (false, 0);
        }

        var probeTemplate =
            new FingerprintTemplate(probeTemplateBytes);

        double bestScore = 0;

        foreach (var stored in templates)
        {
            if (stored.TemplateData == null)
                continue;

            var candidateTemplate =
                new FingerprintTemplate(stored.TemplateData);

            double matchScore = new FingerprintMatcher(probeTemplate)
                .Match(candidateTemplate);

            if (matchScore > bestScore)
                bestScore = matchScore;
        }

        bool success = bestScore >= Threshold;
        int score = (int)Math.Round(bestScore);

        await LogAttempt(
            userId,
            score,
            success);

        Console.WriteLine($"[Service] Verify terminé -> userId={userId}, success={success}, score={score}");

        return (success, score);
    }

    private async Task LogAttempt(int userId,int score,bool success)
    {
        await _authLogRepository.CreateAsync(
            new AuthLog
            {
                UserId = userId,
                Score = score,
                Success = success
            });
    }
}