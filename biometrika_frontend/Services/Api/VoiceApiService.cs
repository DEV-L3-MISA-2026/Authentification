using biometrika.Models;

namespace biometrika.Services;

public class VoiceApiService
{
    public Task<BiometricResponse> ProcessAsync(string userId)
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

            // Simulation: traitement vocal
            return Task.FromResult(new BiometricResponse
            {
                Success = true,
                Message = $"Reconnaissance vocale traitée avec succès (API simulation) pour l'utilisateur {userId}",
                Data = new { UserId = userId, Timestamp = DateTime.UtcNow }
            });
        }
        catch (Exception ex)
        {
            return Task.FromResult(new BiometricResponse
            {
                Success = false,
                Message = $"Erreur API vocale: {ex.Message}"
            });
        }
    }
}