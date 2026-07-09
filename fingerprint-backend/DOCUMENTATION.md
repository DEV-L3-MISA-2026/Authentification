# Documentation de l'API Fingerprint

## Base URL
`https://localhost:5001/api/auth`


# Documentation d'utilisation - Application Fingerprint

## Présentation
Application de capture et vérification d'empreintes digitales via caméra mobile. L'utilisateur peut enrôler une nouvelle empreinte ou vérifier une empreinte existante contre la base de templates.

## Prérequis
- .NET 10 SDK
- PostgreSQL
- Navigateur mobile avec caméra
- Accès réseau au serveur

## Installation
1. Restaurer les packages : `dotnet restore`
2. Appliquer les migrations : `dotnet ef database update`
3. Lancer l'application : `dotnet run`

## Workflow Mobile

### 1. Accès depuis le téléphone
Ouvrir le navigateur sur : `https://<serveur>:5001`

### 2. Enrôler une empreinte
1. Aller sur la liste des utilisateurs
2. Cliquer sur **Enrôler** pour un utilisateur
3. La page demande l'accès à la caméra
4. Positionner le doigt devant la caméra
5. Cliquer sur **Capturer**
6. Cliquer sur **Enregistrer**
7. Le serveur extrait le template et l'enregistre en base
8. Le résultat est affiché

### 3. Vérifier une empreinte
1. Aller sur la liste des utilisateurs
2. Cliquer sur **Vérifier** pour un utilisateur
3. La page demande l'accès à la caméra
4. Positionner le doigt devant la caméra
5. Cliquer sur **Capturer**
6. Cliquer sur **Vérifier**
7. Le serveur compare avec les templates existants
8. Le résultat s'affiche : **Empreinte reconnue** ou **Empreinte non reconnue**

## Architecture technique

### Flux de capture
- Le navigateur mobile accède à la caméra via `getUserMedia`
- L'image est capturée dans un canvas et convertie en PNG base64
- L'image est envoyée au serveur par formulaire POST
- Le serveur extrait le template, enregistre ou vérifie
- Le serveur retourne le résultat

### Points clés
- Capture caméra : **côté client** (navigateur mobile)
- Extraction template : **côté serveur** (`TemplateExtractorService`)
- Vérification : **côté serveur** (`FingerprintService`)
- Stockage : **base de données PostgreSQL**

## Routes MVC
- `GET /Auth/Enroll?userId=1` - Page d'enrôlement
- `POST /Auth/EnrollFromCamera` - Traitement de l'image et enregistrement
- `GET /Auth/Verify?userId=1` - Page de vérification
- `POST /Auth/VerifyFromCamera` - Traitement de l'image et vérification
- `POST /Auth/VerifyFromCameraAjax` - Version AJAX pour vérification

## API REST (optionnelle)
Les endpoints REST sont disponibles pour intégration externe :
- `POST /api/auth/enroll`
- `POST /api/auth/verify`

## Logs de débogage
Consulter le terminal pour les logs :
- `[App]` : démarrage de l'application
- `[Auth]` : actions de capture et traitement
- `[Service]` : opérations de vérification
- `[API]` : appels REST

## Dépannage
- **Caméra inaccessible** : vérifier les permissions du navigateur et utiliser HTTPS
- **Flash non supporté** : certains appareils ne supportent pas le flash
- **Erreur de capture** : vérifier l'éclairage et la position du doigt

## Testeur d'API REST
Un site web de test est disponible dans `wwwroot/apitester/index.html`.
Ouvrez-le dans un navigateur pour tester `enroll` et `verify` :
1. Entrez `username` et `userId`
2. Capturer l'image via la caméra
3. Cliquer sur **Envoyer au serveur**
4. Le serveur traite et retourne le résultat JSON

## Sécurité
- Utiliser HTTPS en production
- Valider les entrées côté serveur
- Authentifier les utilisateurs si nécessaire
