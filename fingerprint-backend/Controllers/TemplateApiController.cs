using Microsoft.AspNetCore.Mvc;
using sourceAFIS_mvc_test.Models;
using sourceAFIS_mvc_test.Repositories;

namespace sourceAFIS_mvc_test.Controllers;

[ApiController]
[Route("api/templates")]
public class TemplateApiController : ControllerBase
{
    private readonly ITemplateRepository _templateRepository;

    public TemplateApiController(
        ITemplateRepository templateRepository)
    {
        _templateRepository = templateRepository;
    }

    [HttpPost]
    public async Task<IActionResult> Create([FromBody] Template template)
    {
        var created = await _templateRepository.CreateAsync(template);

        return CreatedAtAction(
            nameof(Create),
            new { id = created.Id },
            created);
    }
}