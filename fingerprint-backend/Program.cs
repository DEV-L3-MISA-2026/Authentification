using sourceAFIS_mvc_test.Data;
using sourceAFIS_mvc_test.Repositories;
using sourceAFIS_mvc_test.Services;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddControllers()
    .AddJsonOptions(options =>
    {
        options.JsonSerializerOptions.PropertyNamingPolicy = System.Text.Json.JsonNamingPolicy.CamelCase;
    });

// Configuration Swagger/OpenAPI
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Configuration CORS pour autoriser les requêtes du client WASM
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
    {
        policy.AllowAnyOrigin()
              .AllowAnyHeader()
              .AllowAnyMethod();
    });
});

builder.Services.AddDbContext<SourceDbContext>(options =>
    options.UseNpgsql(builder.Configuration.GetConnectionString("DefaultConnection")));

builder.Services.AddScoped<ITemplateRepository, TemplateRepository>();
builder.Services.AddScoped<IAuthLogRepository, AuthLogRepository>();
builder.Services.AddScoped<IFingerprintService, FingerprintService>();
builder.Services.AddScoped<ITemplateExtractorService, TemplateExtractorService>();

var app = builder.Build();

Console.WriteLine("[App] Application démarrée sur " + app.Urls.FirstOrDefault() ?? "http://localhost:5000");

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

if (!app.Environment.IsDevelopment())
{
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseRouting();

// Activer CORS avant Authorization
app.UseCors();

app.UseAuthorization();

app.MapControllers();

app.Run();