using Microsoft.JSInterop;

namespace biometrika.Services;

// Thin wrapper around the JS module. Owns the JS module reference only.
public class VoiceRecorderInterop : IAsyncDisposable
{
    private readonly IJSRuntime _js;
    private IJSObjectReference? _module;

    public VoiceRecorderInterop(IJSRuntime js)
    {
        _js = js;
    }

    private async Task<IJSObjectReference> ModuleAsync()
    {
        _module ??= await _js.InvokeAsync<IJSObjectReference>(
            "import", "./js/voiceRecorder.js");
        return _module;
    }

    public async Task StartAsync(DotNetObjectReference<VoiceRecorderService> selfRef, int maxDurationMs)
    {
        var module = await ModuleAsync();
        await module.InvokeVoidAsync("start", selfRef, maxDurationMs);
    }

    public async Task StopAsync()
    {
        var module = await ModuleAsync();
        await module.InvokeVoidAsync("stop");
    }

    public async Task<bool> SendAsync(string url)
    {
        var module = await ModuleAsync();
        return await module.InvokeAsync<bool>("send", url);
    }

    public async Task ResetAsync()
    {
        var module = await ModuleAsync();
        await module.InvokeVoidAsync("reset");
    }

    public async ValueTask DisposeAsync()
    {
        if (_module is not null)
        {
            await _module.DisposeAsync();
        }
    }
}