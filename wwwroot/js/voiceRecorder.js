// Raw MediaRecorder wrapper. No business logic here, just browser API calls.
let recorder = null;
let chunks = [];
let stream = null;
let recordedBlob = null;
let autoStopTimer = null;

export async function start(dotNetRef, maxDurationMs) {
    stream = await navigator.mediaDevices.getUserMedia({ audio: true });

    recorder = new MediaRecorder(stream);
    chunks = [];
    recordedBlob = null;

    recorder.ondataavailable = (e) => chunks.push(e.data);

    recorder.onstop = () => {
        recordedBlob = new Blob(chunks, { type: recorder.mimeType });
        dotNetRef.invokeMethodAsync("OnRecordingStopped", recordedBlob.size, recordedBlob.type);
    };

    recorder.start();

    autoStopTimer = setTimeout(() => {
        if (recorder && recorder.state === "recording") {
            recorder.stop();
        }
    }, maxDurationMs);
}

export function stop() {
    if (recorder && recorder.state === "recording") {
        recorder.stop();
    }
}

export async function send(url) {
    if (!recordedBlob) {
        throw new Error("No recording available to send");
    }

    const response = await fetch(url, {
        method: "POST",
        body: recordedBlob
    });

    const data = await response.json();

    console.log("score : ", data.score);
    return response.ok;
}

export function reset() {
    if (autoStopTimer) {
        clearTimeout(autoStopTimer);
        autoStopTimer = null;
    }

    chunks = [];
    recordedBlob = null;

    if (stream) {
        stream.getTracks().forEach(t => t.stop());
        stream = null;
    }

    recorder = null;
}