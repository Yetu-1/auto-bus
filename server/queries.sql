CREATE TABLE users(
id SERIAL PRIMARY KEY,
user_id VARCHAR(100) UNIQUE,
username VARCHAR(100) NOT NULL UNIQUE,
password VARCHAR(100),
balance INT
)

CREATE TABLE passengers(
id SERIAL PRIMARY KEY,
uid VARCHAR(100) UNIQUE,
longitude VARCHAR(100),
latitiude VARCHAR(100),
time VARCHAR(100)
)