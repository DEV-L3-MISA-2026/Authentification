
namespace sourceAFIS_mvc_test.Services;

public interface ITemplateExtractorService
{
    Task<byte[]> CreateTemplateAsync(IFormFile fingerprintImage);
    Task<byte[]> CreateTemplateAsync(byte[] imageBytes);
}