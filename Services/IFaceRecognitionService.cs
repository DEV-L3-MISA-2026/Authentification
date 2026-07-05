using biometrika.Models;

namespace biometrika.Services;

public interface IFaceRecognitionService
{
    Task<BiometricResponse> ProcessAsync(string userId);
}
