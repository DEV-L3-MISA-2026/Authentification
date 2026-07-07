namespace sourceAFIS_mvc_test.Models;

public class Template
{
    public int Id{get; set; }
    public int UserId{get; set; }
    public byte []? TemplateData{get; set; }
    public DateTime CreatedAt{get; set; }
}