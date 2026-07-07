# Guide de Test - Biometrika

## Accès à la page de test

Ouvrir dans le navigateur :
```
https://localhost:5001/test-biometrika.html
```

Ou si le serveur Blazor est en cours d'exécution :
```
https://localhost:5001/test-biometrika.html
```

## Architecture

### Backend
- Backend ASP.NET Core avec PostgreSQL (`fingerprint-backend-api/`)
- Port HTTP par défaut : `http://localhost:5000`
- Endpoints biométriques :
  - `POST http://localhost:5000/wasm/fingerprint/enroll` - Enrôler une empreinte
  - `POST http://localhost:5000/wasm/fingerprint/verify` - Vérifier une empreinte
- CORS : autorise toutes origines dans `Program.cs`

### Frontend
- Page HTML autonome avec Vanilla JS
- Stockage local (localStorage) pour les users et templates
- Interface en 3 onglets : Users, Templates, Logs

## Utilisation

### 1. Gérer les Users (onglet "Users")

- **Créer un user** : Remplir le formulaire (nom, email, rôle)
- **Modifier un user** : Cliquer sur "Modifier" dans le tableau
- **Supprimer un user** : Cliquer sur "Supprimer" dans le tableau

### 2. Enrôler une empreinte (onglet "Templates")

1. Sélectionner un utilisateur
2. Choisir une image d'empreinte :
   - **Upload** : Sélectionner un fichier image
   - **Caméra** (si disponible) : Démarrer la caméra, capturer l'image
3. Cliquer sur "Enroller"
4. Le backend crée le template et renvoie un Template ID

### 3. Vérifier une empreinte (onglet "Templates")

1. Sélectionner un utilisateur
2. Sélectionner un template existant pour cet utilisateur
3. Fournir une image d'empreinte (upload ou caméra)
4. Cliquer sur "Vérifier"
5. Le backend renvoie le score et la vérification

### 4. Voir les logs (onglet "Logs")

- Affiche l'historique des vérifications
- Montre le score, le succès/échec, le user et le template

## Test avec images d'empreintes

Pour tester sans scanner :
1. Utiliser des images d'empreintes de test (disponibles dans le dossier `Authentification/build/`)
2. Ou utiliser des images depuis le dataset Fingerprint

## Prérequis Backend

Au préalable, s'assurer que :

1. PostgreSQL est installé et en cours d'exécution
   ```bash
   sudo service postgresql status
   ```

2. La base de données est configurée :
   ```bash
   psql -U postgres -d sourceafis
   CREATE DATABASE sourceafis;
   ```

3. Les migrations EF sont appliquées :
   ```bash
   cd fingerprint-backend-api
   dotnet ef database update
   ```

4. Le backend est démarré :
   ```bash
   dotnet run --launch-profile http
   ```

5. Le frontend statique est accessible via le serveur Blazor :
   ```bash
   dotnet run
   ```
   Puis ouvrir : `https://localhost:5001/test-biometrika.html`

## Résolution de problèmes

### Erreur CORS
Le backend doit autoriser `localhost:5001` et `localhost:5000`.

### Erreur de connexion PostgreSQL
Vérifier le fichier `fingerprint-backend-api/appsettings.json`.

### Backend non accessible
Vérifier que le backend écoute bien sur `http://localhost:5000` (pas HTTPS).