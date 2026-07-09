using System.Net.Http.Json;
using biometrika.Models;
using Microsoft.AspNetCore.Components;

namespace biometrika.Services;

public class FaceApiService
{
    private readonly HttpClient _httpClient;
    private readonly NavigationManager _navigation;
    private readonly string FaceServerUrl;

    public FaceApiService(HttpClient httpClient, NavigationManager navigation)
    {
        _httpClient = httpClient;
        _navigation = navigation;
        
        // Construction dynamique de l'URL du serveur facial
        // Utilise le même hôte que la page actuelle, mais avec le port 5238
        // Cela fonctionne à la fois en localhost (http://localhost:5238) 
        // et depuis un téléphone (http://192.168.x.x:5238)
        var baseUri = new Uri(_navigation.BaseUri);
        FaceServerUrl = $"http://{baseUri.Host}:5238";
        
        Console.WriteLine($"[FaceAPI] URL configurée: {FaceServerUrl} (depuis baseUri: {_navigation.BaseUri})");
    }

    public async Task<BiometricResponse> EnrollAsync(string userId, byte[] faceImage)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(userId) || !int.TryParse(userId, out int userIdInt))
            {
                return new BiometricResponse
                {
                    Success = false,
                    Message = "Format userId invalide. Doit être un nombre entier."
                };
            }

            if (faceImage == null || faceImage.Length == 0)
            {
                return new BiometricResponse
                {
                    Success = false,
                    Message = "Image vide."
                };
            }

            var base64Image = Convert.ToBase64String(faceImage);

            var request = new
            {
                ImageData = $"data:image/jpeg;base64,{base64Image}"
            };

            var response = await _httpClient.PostAsJsonAsync($"{FaceServerUrl}/wasm/face/enroll/{userIdInt}", request);

            if (response.IsSuccessStatusCode)
            {
                var result = await response.Content.ReadFromJsonAsync<WasmFaceResponse>();
                return new BiometricResponse
                {
                    Success = true,
                    Message = $"Template facial enregistré pour l'utilisateur {userId}",
                    Data = new { UserId = userId, TemplateId = result?.TemplateId }
                };
            }
            else
            {
                var error = await response.Content.ReadAsStringAsync();
                return new BiometricResponse
                {
                    Success = false,
                    Message = $"Erreur serveur facial: {error}"
                };
            }
        }
        catch (Exception ex)
        {
            return new BiometricResponse
            {
                Success = false,
                Message = $"Erreur réseau facial: {ex.Message}"
            };
        }
    }

    public async Task<BiometricResponse> VerifyAsync(int userId, byte[] probeImage)
    {
        try
        {
            if (probeImage == null || probeImage.Length == 0)
            {
                return new BiometricResponse
                {
                    Success = false,
                    Message = "Image de vérification vide."
                };
            }

            var base64Image = Convert.ToBase64String(probeImage);

            var request = new
            {
                ImageData = $"data:image/jpeg;base64,{base64Image}"
            };

            var response = await _httpClient.PostAsJsonAsync($"{FaceServerUrl}/wasm/face/verify/{userId}", request);

            if (response.IsSuccessStatusCode)
            {
                var result = await response.Content.ReadFromJsonAsync<WasmFaceResponse>();
                int score = result?.Score ?? 0;
                bool verified = result?.Success ?? false;

                return new BiometricResponse
                {
                    Success = verified,
                    Message = verified
                        ? $"Vérification faciale réussie. Score: {score}"
                        : $"Vérification faciale échouée. Score: {score}",
                    Data = new { UserId = userId, MatchScore = score }
                };
            }
            else
            {
                var error = await response.Content.ReadAsStringAsync();
                return new BiometricResponse
                {
                    Success = false,
                    Message = $"Erreur serveur facial: {error}"
                };
            }
        }
        catch (Exception ex)
        {
            return new BiometricResponse
            {
                Success = false,
                Message = $"Erreur réseau faciale: {ex.Message}"
            };
        }
    }

    private class WasmFaceResponse
    {
        public bool Success { get; set; }
        public string Message { get; set; } = string.Empty;
        public int TemplateId { get; set; }
        public int Score { get; set; }
    }
}