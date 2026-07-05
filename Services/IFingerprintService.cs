using biometrika.Models;

namespace biometrika.Services;

public interface IFingerprintService
{
    Task<BiometricResponse> EnrollAsync(string userId, byte[] fingerprintImage);
    Task<BiometricResponse> VerifyAsync(string userId, byte[] probeImage);
}

public class FingerprintService : IFingerprintService
{
    private const double Threshold = 40;

    public Task<BiometricResponse> EnrollAsync(string userId, byte[] fingerprintImage)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(userId) || !int.TryParse(userId, out _))
            {
                return Task.FromResult(new BiometricResponse
                {
                    Success = false,
                    Message = "Format userId invalide. Doit être un nombre entier."
                });
            }

            if (fingerprintImage == null || fingerprintImage.Length == 0)
            {
                return Task.FromResult(new BiometricResponse
                {
                    Success = false,
                    Message = "Image d'empreinte vide."
                });
            }

            // TODO: Intégrer un algorithme d'extraction de template d'empreinte WASM
            // Pour l'instant, stockage simulé du brut
            
            return Task.FromResult(new BiometricResponse
            {
                Success = true,
                Message = $"Empreinte enregistrée avec succès (simulation) pour l'utilisateur {userId}",
                Data = new { UserId = userId, Timestamp = DateTime.UtcNow }
            });
        }
        catch (Exception ex)
        {
            return Task.FromResult(new BiometricResponse
            {
                Success = false,
                Message = $"Erreur lors de l'enrollment: {ex.Message}"
            });
        }
    }

    public Task<BiometricResponse> VerifyAsync(string userId, byte[] probeImage)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(userId) || !int.TryParse(userId, out _))
            {
                return Task.FromResult(new BiometricResponse
                {
                    Success = false,
                    Message = "Format userId invalide."
                });
            }

            if (probeImage == null || probeImage.Length == 0)
            {
                return Task.FromResult(new BiometricResponse
                {
                    Success = false,
                    Message = "Image de vérification vide."
                });
            }

            // TODO: Intégrer un algorithme de matching WASM
            // Simulation: score aléatoire
            var random = new Random();
            double score = random.Next(0, 100);
            bool verified = score >= Threshold;

            return Task.FromResult(new BiometricResponse
            {
                Success = verified,
                Message = verified 
                    ? $"Vérification réussie (simulation) pour l'utilisateur {userId}"
                    : $"Vérification échouée (simulation). Score: {score:F1}",
                Data = new { UserId = userId, MatchScore = score, Threshold }
            });
        }
        catch (Exception ex)
        {
            return Task.FromResult(new BiometricResponse
            {
                Success = false,
                Message = $"Erreur lors de la vérification: {ex.Message}"
            });
        }
    }
}
