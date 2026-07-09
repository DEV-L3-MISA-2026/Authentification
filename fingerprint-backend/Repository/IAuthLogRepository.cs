using sourceAFIS_mvc_test.Models;

namespace sourceAFIS_mvc_test.Repositories;

public interface IAuthLogRepository
{
    Task<AuthLog> CreateAsync(AuthLog log);
}