# Biometrika - Application Web de Biométrie

Application Blazor WebAssembly pour la capture et le traitement biométrique (empreintes digitales, reconnaissance faciale, reconnaissance vocale).

## Architecture

```
biometrika/                    # Application WebAssembly (client)
├── Pages/
│   ├── Fingerprint/
│   │   ├── FingerprintEnroll.razor   # Enregistrement empreinte
│   │   └── FingerprintVerify.razor   # Vérification empreinte
│   ├── Face/
│   │   └── FaceProcess.razor         # Reconnaissance faciale
│   └── Voice/
│       └── VoiceProcess.razor        # Reconnaissance vocale
├── Services/
│   └── Api/
│       ├── FingerprintApiService.cs  # API SourceAFIS (serveur fingerprint)
│       ├── FaceApiService.cs         # API reconnaissance faciale
│       └── VoiceApiService.cs        # API reconnaissance vocale
└── wwwroot/
    └── js/
        └── fingerprint-camera.js     # Module capture caméra

fingerprint/                   # Serveur ASP.NET (SourceAFIS)
├── Controllers/
│   └── FingerprintWasmController.cs # Endpoints WASM
└── Services/
    └── TemplateExtractorService.cs   # Extraction templates SourceAFIS
```

## Prérequis

- .NET 10 SDK
- Navigateur moderne avec support WebRTC (Chrome, Firefox, Edge)
- Permission caméra/microphone

## Installation et Lancement

### 1. Démarrer le serveur SourceAFIS (fingerprint)

```bash
cd fingerprint
dotnet run
```

Le serveur démarre sur `http://localhost:5000`

### 2. Démarrer l'application WebAssembly (biometrika)

```bash
cd biometrika
dotnet run
```

L'application démarre sur `http://localhost:5284`

### 3. Accéder à l'application

Ouvrir un navigateur et aller sur : `http://localhost:5284`

## Utilisation

### Page d'accueil
- Menu de navigation avec toutes les fonctionnalités
- Accès aux pages : Enr. Empreinte, Vérif. Empreinte, Reconnaissance Vocale, Reconnaissance Faciale

### Enregistrement d'empreinte (`/fingerprint/enroll`)

1. Autoriser l'accès à la caméra
2. Positionner l'empreinte dans le cadre vidéo
3. Cliquer sur **Capturer**
4. L'image est automatiquement envoyée au serveur SourceAFIS
5. Le template est créé et retourné avec un `TemplateId`
6. Le résultat s'affiche : succès/échec + message

**Note :** Aucun utilisateur à créer préalablement. Les templates sont stockés avec un userId par défaut (1).

### Vérification d'empreinte (`/fingerprint/verify`)

1. Autoriser l'accès à la caméra
2. Capturer une empreinte
3. Le serveur compare avec les templates existants
4. Affichage du résultat : `Score de correspondance` (0-100)
5. Seuil de validation : 40

### Reconnaissance faciale (`/face/process`)

- Capture photo via caméra
- Traitement simulé (pas d'intégration DeepFace pour l'instant)

### Reconnaissance vocale (`/voice/process`)

1. Cliquer sur **Enregistrer** (simulation 3 secondes)
2. Le texte est transcrit automatiquement
3. Cliquer sur **Traiter** pour analyser

## Fonctionnalités Techniques

### Capture Caméra
- Module ES6 `fingerprint-camera.js`
- Utilise `navigator.mediaDevices.getUserMedia()`
- Support caméra avant/arrière (selon appareil)
- Capture via canvas → base64
- Torche/flash pour appareils mobiles

### Communication Serveur
- Appels HTTP JSON entre WASM et serveur fingerprint
- Endpoints :
  - `POST http://localhost:5000/wasm/fingerprint/enroll`
  - `POST http://localhost:5000/wasm/fingerprint/verify`
- Fallback simulation locale si serveur inaccessible

### SourceAFIS
- Extraction de templates d'empreintes
- Matching avec score de similarité (0-100)
- Seuil de validation : 40

## Dépannage

### La caméra ne se lance pas
- Vérifier les permissions du navigateur
- Essayer avec HTTPS (getUserMedia requiert HTTPS en production)
- Vérifier la console F12 pour les erreurs

### Le serveur fingerprint n'est pas accessible
- Vérifier que le serveur fingerprint est démarré sur le port 5000
- Vérifier le firewall
- Consulter les logs : `/tmp/fingerprint.log`

### Les boutons ne répondent pas
- Actualiser la page (F5)
- Vider le cache du navigateur
- Vérifier la console JavaScript

## Technologies Utilisées

- **Frontend** : Blazor WebAssembly, Bootstrap 5, Bootstrap Icons
- **Backend** : ASP.NET Core, SourceAFIS
- **Capture média** : JavaScript getUserMedia API
- **Communication** : HTTP/JSON

## Notes

- L'application est en mode développement
- Les données biométriques ne sont pas persistées entre les sessions (pas de base de données dans le client WASM)
- Le serveur fingerprint utilise SQLite/SQL Server pour stocker les templates
- SourceAFIS est incompatible avec WebAssembly d'où l'architecture client-serveur