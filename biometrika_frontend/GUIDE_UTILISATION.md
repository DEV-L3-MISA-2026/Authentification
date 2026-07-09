# 📚 Guide d'Utilisation - Biometrika

## 🎯 Vue d'ensemble

Biometrika est une application de **biométrie multimodale** qui permet d'enregistrer et de vérifier des utilisateurs via :
- **Reconnaissance faciale** (via webcam)
- **Reconnaissance d'empreintes digitales** (WASM)
- **Reconnaissance vocale** (à venir)

---

## 🏗️ Architecture du système

```
┌─────────────────────────────────────────────────────────────┐
│                    Frontend Blazor (Port 5284)               │
│  - Pages Razor (FaceEnroll, FaceVerify, Fingerprint...)     │
│  - Services API (FaceApiService, FingerprintApiService)     │
│  - Capture caméra + preview                                 │
└───────────────────────┬─────────────────────────────────────┘
                        │ HTTP REST API
                        │ http://localhost:5238
└───────────────────────┴─────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│              Backend C++ Crow (Port 5238)                    │
│  - face_api_server.cpp : endpoints /wasm/face/*            │
│  - OpenCV (détection + reconnaissance faciale)              │
│  - Modèles IA : Yunet (détection) + SFace (reconnaissance) │
└───────────────────────┬─────────────────────────────────────┘
                        │ PostgreSQL
└───────────────────────┴─────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    PostgreSQL (Port 5432)                    │
│  Base de données : 'auth'                                   │
│  Table : users (stockage embeddings faciaux)                │
└─────────────────────────────────────────────────────────────┘
```

---

## 📋 Prérequis

### Logiciels requis :
- ✅ **PostgreSQL** (version 12+)
- ✅ **.NET 10 SDK** (ou .NET 8+)
- ✅ **CMake 3.28+**
- ✅ **Compilateur C++** (g++ ou clang++)
- ✅ **OpenCV 4.5+** avec modules contrib
- ✅ **Git**

### Vérifier les installations :

```bash
# PostgreSQL
psql --version

# .NET
dotnet --version

# CMake
cmake --version

# Compilateur
g++ --version

# OpenCV
pkg-config --modversion opencv4
```

---

## ⚙️ Installation et configuration

### Étape 1 : Cloner le projet (si pas déjà fait)

```bash
git clone <url-du-repo>
cd bio
```

### Étape 2 : Configuration PostgreSQL

```bash
# Se connecter à PostgreSQL
sudo -u postgres psql

# Créer la base de données
CREATE DATABASE auth;

# Créer un utilisateur (optionnel, par défaut : user=postgres, password=s)
CREATE USER auth_user WITH PASSWORD 'votre_mot_de_passe';
GRANT ALL PRIVILEGES ON DATABASE auth TO auth_user;

# Quitter
\q
```

### Étape 3 : Configuration Backend C++

```bash
cd Authentification

# Configurer le build (première fois seulement)
mkdir -p build && cd build
cmake ..

# Compiler
make -j4

# Vérifier que l'exécutable est créé
ls -lh app
```

**Configuration de la base de données dans `face_api_server.cpp` :**

Ligne 61 :
```cpp
AuthRepository::init("dbname=auth user=postgres password=s host=localhost port=5432");
```

Modifiez ces paramètres selon votre configuration PostgreSQL.

### Étape 4 : Configuration Frontend Blazor

```bash
cd ../biometrika

# Restaurer les packages NuGet
dotnet restore

# Compiler pour vérifier
dotnet build
```

---

## 🚀 Lancement de l'application

### **Méthode recommandée : Deux terminaux**

#### **Terminal 1 : Backend C++**

```bash
cd Authentification/build
./app
```

**Sortie attendue :**
```
Connexion réussie à la base de données 'auth'.
Starting REST API server on http://0.0.0.0:5238
Endpoints:
  POST /wasm/face/enroll - Register face for a user
  POST /wasm/face/verify - Verify face against registered user
(2026-07-07 10:07:52) [INFO    ] Crow/master server is running at http://0.0.0.0:5238 using 4 threads
```

⚠️ **Important** : Laissez ce terminal ouvert.

---

#### **Terminal 2 : Frontend Blazor**

```bash
cd biometrika
dotnet run
```

**Sortie attendue :**
```
Building...
biometrika -> /home/.../biometrika/bin/Debug/net10.0/biometrika.dll
Listening on: http://localhost:5284
```

⚠️ **Important** : Laissez ce terminal ouvert.

---

## 🌐 Accès à l'application

### URLs principales :

| Page | URL | Description |
|------|-----|-------------|
| **Accueil reconnaissance faciale** | http://localhost:5284/face/process | Page de choix enrollment/vérification |
| **Enrollment facial** | http://localhost:5284/face/enroll/1 | Enregistrement d'un visage (remplacer 1 par l'ID utilisateur) |
| **Vérification faciale** | http://localhost:5284/face/verify/1/1 | Vérification (remplacer par user_id/template_id) |
| **Test empreinte digitale** | http://localhost:5284/fingerprint/enroll | Page de test empreinte digitale |

---

## 📖 Utilisation des fonctionnalités

### 1️⃣ Reconnaissance faciale - Enrollment

#### Étapes :

1. **Ouvrir** : http://localhost:5284/face/enroll/1
   - Le `1` est l'ID de l'utilisateur dans la base de données

2. **Autoriser la caméra** quand le navigateur demande la permission

3. **Positionner le visage** dans le cadre
   - Un aperçu en temps réel est affiché
   - Assurez-vous d'avoir un bon éclairage

4. **Cliquer sur "📸 Capturer"**
   - La photo est prise et affichée

5. **Cliquer sur "✅ Enregistrer"**
   - Le système détecte le visage
   - Il extrait les caractéristiques faciales
   - Il enregistre le template dans la base de données

6. **Notez le TemplateId retourné**
   - Exemple : `"TemplateId": 1`
   - Vous en aurez besoin pour la vérification

#### Résultat attendu :

```json
{
  "Success": true,
  "Message": "Template facial enregistré pour l'utilisateur 1",
  "TemplateId": 1
}
```

---

### 2️⃣ Reconnaissance faciale - Vérification

#### Étapes :

1. **Ouvrir** : http://localhost:5284/face/verify/1/1
   - Premier `1` = user_id
   - Deuxième `1` = template_id

2. **Autoriser la caméra**

3. **Positionner le même visage** que lors de l'enrollment

4. **Cliquer sur "📸 Capturer"**

5. **Cliquer sur "🔍 Vérifier"**

6. **Consulter le résultat**
   - Score de similarité (0-100%)
   - Message de réussite ou d'échec

#### Résultats possibles :

**Succès :**
```json
{
  "Success": true,
  "Message": "Vérification faciale réussie",
  "Score": 85,
  "TemplateId": 1
}
```

**Échec :**
```json
{
  "Success": false,
  "Message": "Vérification faciale échouée",
  "Score": 35,
  "TemplateId": 1
}
```

---

### 3️⃣ Reconnaissance d'empreinte digitale

#### Étapes :

1. **Ouvrir** : http://localhost:5284/fingerprint/enroll

2. **Entrer un nom d'utilisateur**

3. **Cliquer sur "🔍 Rechercher un utilisateur"**

4. **Sélectionner un utilisateur**

5. **Capturer l'empreinte** (simulé ou via lecteur)

6. **Enregistrer le template**

#### Vérification :

1. **Ouvrir** : http://localhost:5284/fingerprint/verify

2. **Entrer le nom d'utilisateur**

3. **Capturer une nouvelle empreinte**

4. **Vérifier la correspondance**

---

## 🔧 Dépannage des erreurs courantes

### ❌ Erreur : "Erreur réseau facial: TypeError: NetworkError when attempting to fetch resource"

**Cause** : Le backend C++ n'est pas accessible.

**Solutions** :

1. **Vérifier que le backend C++ est démarré** (Terminal 1)
   ```bash
   # Doit afficher "Starting REST API server on http://0.0.0.0:5238"
   cd Authentification/build
   ./app
   ```

2. **Vérifier que le port 5238 n'est pas utilisé**
   ```bash
   lsof -ti:5238 | xargs -r kill -9
   ```

3. **Tester la connexion manuellement**
   ```bash
   curl http://localhost:5238/health
   # Doit retourner "OK"
   ```

4. **Vérifier les logs du backend** (Terminal 1)
   - Regardez si les requêtes arrivent bien
   - Vérifiez les erreurs CORS

---

### ❌ Erreur : "PostgreSQL connection failed"

**Cause** : Impossible de se connecter à la base de données.

**Solutions** :

1. **Vérifier que PostgreSQL est démarré**
   ```bash
   sudo systemctl status postgresql
   sudo systemctl start postgresql
   ```

2. **Vérifier les paramètres de connexion** dans `face_api_server.cpp` :
   ```cpp
   AuthRepository::init("dbname=auth user=postgres password=s host=localhost port=5432");
   ```

3. **Tester la connexion manuellement**
   ```bash
   psql -U postgres -d auth
   ```

---

### ❌ Erreur : "No face detected in image"

**Cause** : Le système n'a pas détecté de visage dans la photo.

**Solutions** :

1. **Améliorer l'éclairage**
2. **Positionner le visageface à la caméra**
3. **Éviter les ombres sur le visage**
4. **Utiliser une photo nette et bien cadrée**

---

### ❌ Erreur : "Access Denied" pour la caméra

**Cause** : Le navigateur a bloqué l'accès à la caméra.

**Solutions** :

1. **Autoriser la caméra dans le navigateur**
   - Cliquez sur l'icône de caméra dans la barre d'adresse
   - Sélectionnez "Autoriser"

2. **Vérifier les paramètres du navigateur**
   - Chrome : Paramètres → Confidentialité et sécurité → Paramètres du site → Caméra
   - Firefox : Paramètres → Vie privée et sécurité → Autorisations → Caméra

---

### ❌ Erreur : "Failed to decode image" (400)

**Cause** : L'image capturée est invalide ou corrompue.

**Solutions** :

1. **Vérifier que la caméra fonctionne**
   ```bash
   # Tester avec Cheese ou VLC
   cheese
   ```

2. **Redémarrer le navigateur**

---

### ❌ Erreur CORS (Cross-Origin Resource Sharing)

**Cause** : Le frontend et le backend sont sur des ports différents.

**Solution** : ✅ **Déjà résolu** dans le code actuel. Les headers CORS sont correctement configurés.

Si le problème persiste, vérifiez dans la console développeur du navigateur (F12) :
- Onglet **Console** : messages d'erreur CORS
- Onglet **Réseau** : statut des requêtes OPTIONS

---

## 🧪 Tests de validation

### Test 1 : Vérifier que le backend fonctionne

```bash
# Health check
curl http://localhost:5238/health
# Réponse attendue : "OK"

# Test OPTIONS (CORS preflight)
curl -X OPTIONS http://localhost:5238/wasm/face/enroll \
  -H "Origin: http://localhost:5284" \
  -H "Access-Control-Request-Method: POST" \
  -I
# Réponse attendue : HTTP/1.1 204 No Content
```

### Test 2 : Vérifier que le frontend fonctionne

```bash
# Accéder à la page
curl http://localhost:5284/face/process
# Doit retourner le HTML de la page
```

### Test 3 : Test complet d'enrollment

```bash
# 1. Créer un utilisateur test dans PostgreSQL
sudo -u postgres psql auth
# INSERT INTO users (username, password_hash) VALUES ('test_user', 'hash');

# 2. Récupérer l'user_id
# SELECT id FROM users WHERE username = 'test_user';

# 3. Ouvrir le navigateur
# http://localhost:5284/face/enroll/{user_id}

# 4. Capturer un visage et enregistrer
# 5. Noter le TemplateId retourné
```

### Test 4 : Test complet de vérification

```bash
# 1. Ouvrir le navigateur
# http://localhost:5284/face/verify/{user_id}/{template_id}

# 2. Capturer le même visage
# 3. Cliquer sur Vérifier
# 4. Vérifier le score de similarité
```

---

## 📊 Structure de la base de données

### Table `users`

```sql
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    facial_embeddings FLOAT[],
    fingerprint_template BYTEA,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Champs importants :

- **facial_embeddings** : Tableau de 128 floats (embedding facial SFace)
- **fingerprint_template** : Template brut de l'empreinte (WASM)
- **password_hash** : Mot de passe hashé (pour authentification)

---

## 🔍 Exemples de cas d'usage

### Cas 1 : Enrollment d'un nouvel utilisateur

```
1. Admin crée un utilisateur dans la base de données
   → user_id = 5, username = "jean_dupont"

2. Jean va sur http://localhost:5284/face/enroll/5

3. Jean capture son visage

4. Système retourne : { "Success": true, "TemplateId": 5 }

5. Admin note : user_id=5, template_id=5
```

### Cas 2 : Vérification à l'accès

```
1. Jean va sur http://localhost:5284/face/verify/5/5

2. Jean capture son visage

3. Système compare avec le template stocké

4. Si similarité > 0.5 :
   → Accès autorisé (Score: 85%)
```

### Cas 3 : Test multi-utilisateurs

```
Utilisateur 1 : user_id=1, template_id=1
Utilisateur 2 : user_id=2, template_id=2
Utilisateur 3 : user_id=3, template_id=3

Chaque utilisateur a son propre template facial.
Le système vérifie uniquement contre le template de l'utilisateur connecté.
```

---

## 🎨 Interface utilisateur

### Page d'accueil (FaceProcess.razor)

```
┌─────────────────────────────────────────┐
│   Biometrika - Reconnaissance Faciale   │
├─────────────────────────────────────────┤
│                                         │
│   👤 Enregistrer un visage              │
│   → Enrollment facial                   │
│                                         │
│   🔍 Vérifier un visage                 │
│   → Vérification faciale                │
│                                         │
└─────────────────────────────────────────┘
```

### Page d'enrollment (FaceEnroll.razor)

```
┌─────────────────────────────────────────┐
│ Enrollment Facial - Utilisateur #1     │
├─────────────────────────────────────────┤
│                                         │
│  ┌─────────────────────────────────┐    │
│  │                                 │    │
│  │   📷 Aperçu caméra              │    │
│  │                                 │    │
│  └─────────────────────────────────┘    │
│                                         │
│  [📸 Capturer] [💾 Enregistrer]        │
│                                         │
│  Miniature de la photo capturée         │
│                                         │
│  Résultat : { Success: true, ... }      │
│                                         │
└─────────────────────────────────────────┘
```

### Page de vérification (FaceVerify.razor)

```
┌─────────────────────────────────────────┐
│ Vérification Faciale - User #1         │
├─────────────────────────────────────────┤
│                                         │
│  ┌─────────────────────────────────┐    │
│  │                                 │    │
│  │   📷 Aperçu caméra              │    │
│  │                                 │    │
│  └─────────────────────────────────┘    │
│                                         │
│  [📸 Capturer] [🔍 Vérifier]           │
│                                         │
│  Miniature de la photo                  │
│                                         │
│  Résultat : { Score: 85, ... }          │
│                                         │
└─────────────────────────────────────────┘
```

---

## ⚡ Optimisation des performances

### Backend C++

**Multi-threading** :
```cpp
app.port(5238).multithreaded().run();
```
Le serveur utilise 4 threads par défaut (détecté automatiquement).

**Modèles IA** :
- **Détection** : Yunet 2023mar (rapide, léger)
- **Reconnaissance** : SFace 2021dec (précis, rapide)

**Taille d'image** : 320x320 (optimisé pour la vitesse)

### Frontend Blazor

**Compression** : Les images sont envoyées en base64 (JPEG compressé).

**Preview** : Aperçu en temps réel sans sauvegarde (WebRTC).

---

## 🐛 Debug et logs

### Activer les logs détaillés

**Backend C++** :
```cpp
// Dans main.cpp
CROW_LOG_INFO << "Requête reçue: " << req.method_name();
```

**Frontend Blazor** :
```csharp
// Dans FaceApiService.cs
Console.WriteLine($"[FaceAPI] URL configurée: {FaceServerUrl}");
```

### Console développeur navigateur (F12)

- **Onglet Console** : Messages JavaScript, erreurs fetch
- **Onglet Réseau** : Détails des requêtes HTTP
- **Onglet Sécurité** : Problèmes CORS

### Logs PostgreSQL

```bash
# Activer les logs
sudo -u postgres psql
ALTER SYSTEM SET log_statement = 'all';
ALTER SYSTEM SET log_destination = 'stderr';
SELECT pg_reload_conf();
```

---

## 📦 Déploiement en production

### Backend C++

```bash
# Compiler en release
cd Authentification/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

# Lancer en arrière-plan
./app &
```

### Frontend Blazor

```bash
# Publier
cd biometrika
dotnet publish -c Release -o ./publish

# Servir avec Nginx ou Apache
# Ou utiliser dotnet directement
dotnet ./publish/biometrika.dll
```

### Configuration Nginx (exemple)

```nginx
server {
    listen 80;
    server_name votre-domaine.com;

    # Frontend Blazor
    location / {
        proxy_pass http://localhost:5284;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection keep-alive;
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }

    # API Backend C++
    location /api/ {
        proxy_pass http://localhost:5238/;
        proxy_set_header Host $host;
    }
}
```

---

## 🔒 Sécurité

### Recommandations :

1. **HTTPS en production**
   - Certificat SSL/TLS (Let's Encrypt)
   - Redirection HTTP → HTTPS

2. **Authentification**
   - JWT tokens pour les API
   - Validation des inputs (déjà implémentée)

3. **Rate limiting**
   - Limiter les tentatives de reconnaissance
   - Protection contre les attaques par force brute

4. **Validation des images**
   - Vérifier la taille max (déjà implémentée)
   - Vérifier le format (déjà implémenté)
   - Scan antivirus (optionnel)

5. **CORS**
   - Remplacer `Access-Control-Allow-Origin: *` par votre domaine
   ```cpp
   res.set_header("Access-Control-Allow-Origin", "https://votre-domaine.com");
   ```

---

## 🎓 Glossaire

- **Template** : représentation numérique d'une biométrie (visage, empreinte)
- **Embedding** : vecteur de caractéristiques extrait par IA (128 dimensions pour SFace)
- **Similarité cosinus** : mesure de similarité entre deux embeddings (0 à 1)
- **Seuil** : valeur minimum pour accepter une vérification (0.5 = 50%)
- **WASM** : WebAssembly, permet d'exécuter du code C++ dans le navigateur
- **CORS** : Cross-Origin Resource Sharing, autorise les requêtes entre domaines

---

## 📞 Support

En cas de problème :

1. **Consulter ce guide** : Section "Dépannage"
2. **Vérifier les logs** : Backend, Frontend, Navigateur
3. **Tester les connexions** : curl, psql
4. **Redémarrer les services** : Backend + Frontend

---

## 🎉 Félicitations !

Vous avez maintenant une application de **biométrie multimodale** complète et fonctionnelle.

**Fonctionnalités disponibles** :
- ✅ Reconnaissance faciale (enrollment + vérification)
- ✅ Capture caméra avec preview
- ✅ Interface web intuitive
- ✅ API REST performante
- ✅ Base de données PostgreSQL

**Améliorations possibles** :
- 🔄 Reconnaissance vocale
- 🔄 Authentification à deux facteurs
- 🔄 Dashboard admin
- 🔄 Export/import de templates
- 🔄 Multi-langue

---

*Documentation générée le 7 Juillet 2026*