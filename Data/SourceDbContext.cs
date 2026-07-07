using Microsoft.EntityFrameworkCore;
namespace sourceAFIS_mvc_test.Data;

public class SourceDbContext : DbContext
{
    public SourceDbContext(DbContextOptions<SourceDbContext> options) : base(options)
    {
        
    }
    // Users removed - templates only store UserId as int
    public DbSet<Models.Template> Templates { get; set; }
    public DbSet<Models.AuthLog> AuthLogs { get; set; }
}