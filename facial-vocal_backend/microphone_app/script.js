let recorder;
let chunks = [];
let stream;
let recordedBlob = null;
let timer = null;

const startBtn = document.getElementById("start");
const stopBtn = document.getElementById("stop");
const sendBtn = document.getElementById("send");
const resetBtn = document.getElementById("reset");
const status = document.getElementById("status");

startBtn.onclick = async () => {
    stream = await navigator.mediaDevices.getUserMedia({ audio: true });

    recorder = new MediaRecorder(stream);
    chunks = [];
    recordedBlob = null;

    recorder.ondataavailable = (e) => {
        chunks.push(e.data);
    };

    recorder.onstop = () => {
        recordedBlob = new Blob(chunks, { type: recorder.mimeType });

        sendBtn.disabled = false;
        resetBtn.disabled = false;
        status.textContent = "Recorded (ready to send)";
    };

    recorder.start();
    status.textContent = "Recording... (max 3s)";

    startBtn.disabled = true;
    stopBtn.disabled = false;

    // auto-stop after 3 seconds
    timer = setTimeout(() => {
        if (recorder.state === "recording") {
            recorder.stop();
        }
    }, 3000);
};

stopBtn.onclick = () => {
    if (recorder && recorder.state === "recording") {
        recorder.stop();
    }
};

sendBtn.onclick = async () => {
    if (!recordedBlob) return;

    const form = new FormData();
    form.append("audio", recordedBlob, "voice.webm");

    status.textContent = "Sending...";
    console.log(recordedBlob);
    console.log(recordedBlob.size);
    console.log(recordedBlob.type);
await fetch("http://localhost:5000/api/voc/test", {
    method: "POST",
    body: recordedBlob
});

    status.textContent = "Sent";

    sendBtn.disabled = true;
};

resetBtn.onclick = () => {

    if (timer) clearTimeout(timer);

    chunks = [];
    recordedBlob = null;

    startBtn.disabled = false;
    stopBtn.disabled = true;
    sendBtn.disabled = true;
    resetBtn.disabled = true;

    status.textContent = "Idle";

    if (stream) {
        stream.getTracks().forEach(t => t.stop());
    }
};