using sourceAFIS_mvc_test.Data;
using sourceAFIS_mvc_test.Models;

namespace sourceAFIS_mvc_test.Repositories;

public class AuthLogRepository : IAuthLogRepository
{
    private readonly SourceDbContext _context;

    public AuthLogRepository(SourceDbContext context)
    {
        _context = context;
    }

    public async Task<AuthLog> CreateAsync(AuthLog log)
    {
        log.CreatedAt = DateTime.UtcNow;

        _context.AuthLogs.Add(log);
        await _context.SaveChangesAsync();

        return log;
    }
}