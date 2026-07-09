using SourceAFIS;

namespace sourceAFIS_mvc_test.Services;

public class TemplateExtractorService : ITemplateExtractorService
{
    public async Task<byte[]> CreateTemplateAsync(
        IFormFile fingerprintImage)
    {
        using var stream = new MemoryStream();

        await fingerprintImage.CopyToAsync(stream);

        byte[] imageBytes = stream.ToArray();

        var image = new FingerprintImage(imageBytes);

        var template = new FingerprintTemplate(image);

        return template.ToByteArray();
    }
    public async Task<byte[]> CreateTemplateAsync(byte[] imageBytes)
{
    var image =
        new FingerprintImage(imageBytes);

    var template =
        new FingerprintTemplate(image);

    return template.ToByteArray();
}
}