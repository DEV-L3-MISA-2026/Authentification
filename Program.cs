using Microsoft.AspNetCore.Components.Web;
using Microsoft.AspNetCore.Components.WebAssembly.Hosting;
using biometrika;
using biometrika.Services;

var builder = WebAssemblyHostBuilder.CreateDefault(args);
builder.RootComponents.Add<App>("#app");
builder.RootComponents.Add<HeadOutlet>("head::after");

builder.Services.AddScoped(sp => new HttpClient { BaseAddress = new Uri(builder.HostEnvironment.BaseAddress) });

// Enregistrement des services biométriques
builder.Services.AddScoped<IFingerprintService, FingerprintService>();
builder.Services.AddScoped<IVoiceRecognitionService, VoiceRecognitionService>();
builder.Services.AddScoped<IFaceRecognitionService, FaceRecognitionService>();

// Enregistrement des services API-style (pour usage direct dans les pages)
builder.Services.AddScoped<FingerprintApiService>();
builder.Services.AddScoped<VoiceApiService>();
builder.Services.AddScoped<FaceApiService>();


builder.Services.AddScoped<VoiceRecorderService>();
builder.Services.AddScoped<VoiceRecorderInterop>();

await builder.Build().RunAsync();
