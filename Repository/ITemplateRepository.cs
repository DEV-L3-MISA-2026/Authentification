using sourceAFIS_mvc_test.Models;

namespace sourceAFIS_mvc_test.Repositories
{
    public interface ITemplateRepository
    {
        Task<List<Template>> GetAllAsync();
        Task<Template?> GetByIdAsync(int id);
        Task<List<Template>> GetByUserIdAsync(int userId);

        Task<Template> CreateAsync(Template template);
        Task<bool> DeleteAsync(int id);
    }
}