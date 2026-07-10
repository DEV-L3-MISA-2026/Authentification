using Microsoft.JSInterop;

namespace biometrika.Services;

public enum RecordingState
{
    Idle,
    Recording,
    Ready,   // recording stopped, blob available, not sent yet
    Sending,
    Sent
}

// Owns recording state and drives the interop layer.
public class VoiceRecorderService : IAsyncDisposable
{
    private const int MaxDurationMs = 3000;

    private readonly VoiceRecorderInterop _interop;
    private DotNetObjectReference<VoiceRecorderService>? _selfRef;

    public RecordingState State { get; private set; } = RecordingState.Idle;
    public event Action? StateChanged;

    public VoiceRecorderService(VoiceRecorderInterop interop)
    {
        _interop = interop;
    }

    public async Task StartAsync()
    {
        if (State is RecordingState.Recording) return;

        _selfRef ??= DotNetObjectReference.Create(this);
        await _interop.StartAsync(_selfRef, MaxDurationMs);

        State = RecordingState.Recording;
        StateChanged?.Invoke();
    }

    public async Task StopAsync()
    {
        if (State is not RecordingState.Recording) return;

        await _interop.StopAsync();
        // state moves to Ready via OnRecordingStopped, once the blob is built
    }

    [JSInvokable]
    public void OnRecordingStopped(long blobSize, string blobType)
    {
        State = RecordingState.Ready;
        StateChanged?.Invoke();
    }

    public async Task SendAsync(string url)
    {
        if (State is not RecordingState.Ready) return;

        State = RecordingState.Sending;
        StateChanged?.Invoke();

        var ok = await _interop.SendAsync(url);

        State = ok ? RecordingState.Sent : RecordingState.Ready;
        StateChanged?.Invoke();
    }

    public async Task ResetAsync()
    {
        await _interop.ResetAsync();
        State = RecordingState.Idle;
        StateChanged?.Invoke();
    }

    public async ValueTask DisposeAsync()
    {
        _selfRef?.Dispose();
        await _interop.ResetAsync();
        await _interop.DisposeAsync();
    }
}