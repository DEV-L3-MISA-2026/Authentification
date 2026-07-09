# fingerprint

Application de capture et vérification d'empreintes digitales via caméra mobile.

## Démarrage rapide
1. `dotnet restore`
2. `dotnet ef database update`
3. `dotnet run`
4. Ouvrir `https://localhost:5001`

## Usage
- Liste utilisateurs : `User/Index`
- Enrôler : `Auth/Enroll?userId=1`
- Vérifier : `Auth/Verify?userId=1`
- Consultation logs : `Auth/Logs`

## Documentation complète
Voir `DOCUMENTATION.md`.
