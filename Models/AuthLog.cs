namespace sourceAFIS_mvc_test.Models;

public class AuthLog
{
    public int Id{get; set; }
    public int UserId{get; set; }
    public int Score{get; set; }
    public bool Success{get; set; }
    public DateTime CreatedAt{get; set; }
}