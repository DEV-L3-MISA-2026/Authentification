namespace sourceAFIS_mvc_test.DTOs;

public class EnrollRequest
{
    public string Username { get; set; } = string.Empty;
    public int UserId { get; set; }
    public byte[] TemplateBytes { get; set; } = Array.Empty<byte>();
    public string ImageData { get; set; } = string.Empty;
}