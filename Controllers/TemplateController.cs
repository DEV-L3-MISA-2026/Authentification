using Microsoft.AspNetCore.Mvc;
using sourceAFIS_mvc_test.Models;
using sourceAFIS_mvc_test.Repositories;

namespace sourceAFIS_mvc_test.Controllers
{
    public class TemplateController : Controller
    {
        private readonly ITemplateRepository _templateRepository;

        public TemplateController(ITemplateRepository templateRepository)
        {
            _templateRepository = templateRepository;
        }

        public async Task<IActionResult> Index()
        {
            var templates = await _templateRepository.GetAllAsync();
            return View(templates);
        }

        public async Task<IActionResult> Details(int id)
        {
            var template = await _templateRepository.GetByIdAsync(id);
            if (template == null) return NotFound();

            return View(template);
        }

        public IActionResult Create()
        {
            return View();
        }

        [HttpPost]
        public async Task<IActionResult> Create(Template template)
        {
            if (!ModelState.IsValid)
                return View(template);

            await _templateRepository.CreateAsync(template);
            return RedirectToAction(nameof(Index));
        }

        public async Task<IActionResult> Delete(int id)
        {
            var template = await _templateRepository.GetByIdAsync(id);
            if (template == null) return NotFound();

            return View(template);
        }

        [HttpPost, ActionName("Delete")]
        public async Task<IActionResult> DeleteConfirmed(int id)
        {
            await _templateRepository.DeleteAsync(id);
            return RedirectToAction(nameof(Index));
        }
    }
}