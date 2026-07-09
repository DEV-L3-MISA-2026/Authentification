namespace sourceAFIS_mvc_test.Services
{
    public interface IFingerprintService
    {
        Task<(bool Success, int Score)> VerifyAsync(byte[] probeTemplate, int userId);
    }
}