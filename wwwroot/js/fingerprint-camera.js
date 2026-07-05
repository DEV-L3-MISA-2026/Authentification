// Fingerprint camera module - ES6 module for Blazor JS Interop
let videoElement = null;
let mediaStream = null;

// Try a list of constraint sets in order until one succeeds.
// On mobile, we use 'user' facing mode to avoid inversion issues
async function getStreamWithFallback() {
    const constraintsList = [
        { video: { facingMode: { ideal: 'user' }, width: { ideal: 640 }, height: { ideal: 480 } }, audio: false },
        { video: { facingMode: { ideal: 'environment' }, width: { ideal: 640 }, height: { ideal: 480 } }, audio: false },
        { video: true, audio: false } // last resort: whatever camera is available
    ];

    let lastError = null;
    for (const constraints of constraintsList) {
        try {
            return await navigator.mediaDevices.getUserMedia(constraints);
        } catch (err) {
            lastError = err;
            // Only fall through to the next option for constraint-related
            // failures; permission/hardware errors won't be fixed by retrying.
            if (err.name !== 'OverconstrainedError' && err.name !== 'ConstraintNotSatisfiedError') {
                throw err;
            }
        }
    }
    throw lastError;
}

console.log('fingerprint-camera.js loaded');

export async function initCamera(videoId) {
    try {
        const video = document.getElementById(videoId);
        if (!video) {
            console.error('Video element not found:', videoId);
            return { success: false, message: 'Élément vidéo introuvable' };
        }

        if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
            const secure = window.isSecureContext;
            const msg = secure
                ? 'getUserMedia non supporté par ce navigateur'
                : 'La caméra nécessite une connexion HTTPS (ou localhost). Cette page est chargée en HTTP non sécurisé.';
            return { success: false, message: msg };
        }

        // Stop existing stream
        if (mediaStream) {
            mediaStream.getTracks().forEach(t => t.stop());
        }

        const stream = await getStreamWithFallback();

        videoElement = video;
        mediaStream = stream;
        // Muting is required for autoplay to be allowed on iOS Safari and some
        // Android in-app browsers. Without this, video.play() can be silently
        // rejected even though the camera permission was granted.
        video.muted = true;
        video.srcObject = stream;

        try {
            await video.play();
        } catch (playErr) {
            // Keep this separate from the getUserMedia error handling below —
            // the stream itself is fine here, only playback was blocked, so
            // don't let it get mislabeled as a permission-denied error.
            console.error('video.play() error:', playErr);
            return {
                success: false,
                message: 'Flux caméra obtenu mais lecture vidéo bloquée par le navigateur (' + (playErr.name || playErr.message) + '). Réessayez ou utilisez un autre navigateur.'
            };
        }

        console.log('Camera initialized successfully');
        return { success: true, message: 'Caméra prête' };
    } catch (err) {
        console.error('Camera init error:', err);
        let msg = err.message || String(err);
        switch (err.name) {
            case 'NotAllowedError':
            case 'PermissionDeniedError':
                msg = 'Permission caméra refusée. Autorisez l\'accès dans les paramètres du navigateur.';
                break;
            case 'NotFoundError':
            case 'DevicesNotFoundError':
                msg = 'Aucune caméra trouvée sur cet appareil.';
                break;
            case 'OverconstrainedError':
            case 'ConstraintNotSatisfiedError':
                msg = 'Aucune caméra ne correspond aux critères demandés.';
                break;
            case 'NotReadableError':
            case 'TrackStartError':
                msg = 'La caméra est déjà utilisée par une autre application.';
                break;
        }
        return { success: false, message: msg };
    }
}

export function captureImage() {
    if (!videoElement || !mediaStream) {
        return { success: false, message: 'Caméra non active' };
    }
    try {
        const canvas = document.createElement('canvas');
        canvas.width = videoElement.videoWidth || 640;
        canvas.height = videoElement.videoHeight || 480;
        const ctx = canvas.getContext('2d');
        ctx.drawImage(videoElement, 0, 0, canvas.width, canvas.height);
        const dataUrl = canvas.toDataURL('image/jpeg', 0.9);
        return { success: true, imageData: dataUrl };
    } catch (err) {
        console.error('Capture error:', err);
        return { success: false, message: err.message || String(err) };
    }
}

export function stopCamera() {
    setTorch(false);
    if (mediaStream) {
        mediaStream.getTracks().forEach(t => t.stop());
        mediaStream = null;
    }
    if (videoElement) {
        videoElement.srcObject = null;
    }
    console.log('Camera stopped');
}

export function setDotNetHelper(ref) {
    window.dotNetHelper = ref;
}

window.fingerprintCamera = { initCamera, captureImage, stopCamera, setTorch, setDotNetHelper };

export async function setTorch(enabled) {
    if (!mediaStream) {
        return { success: false, message: 'Caméra non active' };
    }
    const track = mediaStream.getVideoTracks()[0];
    if (!track) {
        return { success: false, message: 'Aucune piste vidéo' };
    }
    const capabilities = track.getCapabilities?.();
    if (!capabilities?.torch) {
        return { success: false, message: 'Flash non supporté sur cet appareil' };
    }
    try {
        await track.applyConstraints({ advanced: [{ torch: enabled }] });
        return { success: true };
    } catch (err) {
        console.error('Torch error:', err);
        return { success: false, message: err.message || String(err) };
    }
}
