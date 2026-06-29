#include <FacesModel.h>
#include <LocalException.h>
#include <utility.h>

std::shared_ptr<FacesModel> FacesModel::instance = nullptr;
FacesModel::FacesModel() 
    : conn("dbname=auth user=postgres password=mamanlah host=localhost port=5432") // <- ICI
{
    std::cout << "Connexion réussie à la base de données 'auth'." << std::endl;
}

std::shared_ptr<FacesModel> FacesModel::GetInstance() 
{
    if (!instance)
        instance = std::shared_ptr<FacesModel>(new FacesModel);

    return instance;
}

int FacesModel::getIdByUsername(const std::string& username) 
{
    pqxx::nontransaction tx(this->conn);

    std::string sql = 
        "SELECT id FROM users WHERE username=$1";
    
    pqxx::result rows = tx.exec_params(sql, username);

    if (rows.size() == 0)
        return -1;
    else
        return rows[0][0].as<int>();
}

std::vector<float> FacesModel::formatEmbending(cv::Mat matEmbending)
{
    std::vector<float> embending(128);
    if(matEmbending.size().width != 128 || matEmbending.size().height != 1)
        throw LocalException("bad cv::mat embending shape !");

    float *row_ptr = matEmbending.ptr<float>(0); // pointor at the first row
    for(int i = 0; i < 128; i++)
        embending[i] = row_ptr[i];
    return embending;
}

// inserting the face embending in the db
void FacesModel::insertFaceEmbending(const std::string& username, std::vector<float> embending) {
    int id = this->getIdByUsername(username);
    if (id == -1)
        throw LocalException("cannot find the username in the db !");

    std::string embending_str;
    if (embending.size() != 128)
        throw LocalException("bad face embending shape, we need a 128 vector!");
    embending_str = vector_to_pgstring(embending); // changing the vector to match the format [...] of postgres
    
    // com with the db
    pqxx::work tx(this->conn);
    std::string sql = "INSERT INTO auth_data (userid, face_embending, voice_embending) VALUES ($1, $2, $3)";
    std::vector<float> static_voice_embending(192, 1); 
    std::string static_voice = vector_to_pgstring(static_voice_embending);
    try {

        tx.exec_params(sql, id, embending_str, static_voice);
        tx.commit(); 

    } catch (const pqxx::sql_error& err) {
        std::string detail_erreur = "Erreur SQL : " + std::string(err.what()) + "\n" +
                                    "Requête exécutée : " + err.query();
        throw std::runtime_error(detail_erreur);

    } catch (const std::exception& err) {
        throw std::runtime_error("Erreur standard : " + std::string(err.what()));
    }
}