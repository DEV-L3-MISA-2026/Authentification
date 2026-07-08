-- Initialisation de la base unifiée 'biometrika' regroupant
-- la partie faciale/voix (C++) et la partie empreinte (C#).
--
-- Usage :
--   PGPASSWORD=s psql -U postgres -h localhost -f Authentification/sql/init_biometrika.sql
--
-- Note : les embeddings sont stockés dans des colonnes real[] (tableau
-- PostgreSQL natif) au format texte '[...]' produit par vector_to_pgstring.
-- Ceci évite la dépendance à l'extension pgvector, tout en restant 100%
-- compatible avec le code C++ existant (la similarité est calculée en C++,
-- pas via les opérateurs SQL de pgvector). Si pgvector est installé plus
-- tard, ces colonnes pourront être castées en vector(n).

-- 1) Création de la base (ignoré si elle existe déjà)
SELECT 'CREATE DATABASE biometrika'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'biometrika')\gexec

-- 2) Le reste s'exécute dans la base biometrika
\c biometrika

-- 3) Table users (partie C++)
CREATE TABLE IF NOT EXISTS public.users (
    id integer NOT NULL,
    username character varying(50)
);
ALTER TABLE public.users OWNER TO postgres;

CREATE SEQUENCE IF NOT EXISTS public.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;
ALTER TABLE public.users_id_seq OWNER TO postgres;

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;
ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);
ALTER TABLE ONLY public.users ADD CONSTRAINT users_pkey PRIMARY KEY (id);

-- 4) Table auth_data (embeddings face 128 + voix 192, stockés en real[])
CREATE TABLE IF NOT EXISTS public.auth_data (
    id integer NOT NULL,
    userid integer NOT NULL,
    face_embendding real[],
    voice_embendding real[]
);
ALTER TABLE public.auth_data OWNER TO postgres;

CREATE SEQUENCE IF NOT EXISTS public.auth_data_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;
ALTER TABLE public.auth_data_id_seq OWNER TO postgres;

CREATE SEQUENCE IF NOT EXISTS public.auth_data_userid_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;
ALTER TABLE public.auth_data_userid_seq OWNER TO postgres;

ALTER SEQUENCE public.auth_data_id_seq OWNED BY public.auth_data.id;
ALTER SEQUENCE public.auth_data_userid_seq OWNED BY public.auth_data.userid;

ALTER TABLE ONLY public.auth_data ALTER COLUMN id SET DEFAULT nextval('public.auth_data_id_seq'::regclass);
ALTER TABLE ONLY public.auth_data ALTER COLUMN userid SET DEFAULT nextval('public.auth_data_userid_seq'::regclass);

ALTER TABLE ONLY public.auth_data ADD CONSTRAINT auth_data_userid_fkey
    FOREIGN KEY (userid) REFERENCES public.users(id);