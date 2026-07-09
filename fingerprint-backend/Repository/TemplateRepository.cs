using Microsoft.EntityFrameworkCore;
using sourceAFIS_mvc_test.Models;
using sourceAFIS_mvc_test.Data;
namespace sourceAFIS_mvc_test.Repositories
{
    public class TemplateRepository : ITemplateRepository
    {
        private readonly SourceDbContext _context;

        public TemplateRepository(SourceDbContext context)
        {
            _context = context;
        }

        public async Task<List<Template>> GetAllAsync()
        {
            return await _context.Templates.ToListAsync();
        }

        public async Task<Template?> GetByIdAsync(int id)
        {
            return await _context.Templates.FirstOrDefaultAsync(x => x.Id == id);
        }

        public async Task<List<Template>> GetByUserIdAsync(int userId)
        {
            return await _context.Templates
                .Where(x => x.UserId == userId)
                .ToListAsync();
        }

        public async Task<Template> CreateAsync(Template template)
        {
            template.CreatedAt = DateTime.UtcNow;

            _context.Templates.Add(template);
            await _context.SaveChangesAsync();

            return template;
        }

        public async Task<bool> DeleteAsync(int id)
        {
            var template = await _context.Templates.FirstOrDefaultAsync(x => x.Id == id);

            if (template == null)
                return false;

            _context.Templates.Remove(template);
            await _context.SaveChangesAsync();

            return true;
        }
    }
}