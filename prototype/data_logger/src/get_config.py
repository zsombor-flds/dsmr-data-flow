import os


def load_envvar(envar_name) -> str:
    """
    Load enviroment variable
    """
    if envar_name not in os.environ:
        raise ValueError(f"Missing Environment variable: {envar_name}")
    return os.getenv(envar_name)


def read_config() -> dict:
    """
    Create configuration dict
    """
    config = {}

    config["pg_username"] = load_envvar("PG_DB_USERNAME")
    config["pg_password"] = load_envvar("PG_DB_PASSWORD")
    config["pg_db"] = load_envvar("PG_DB_NAME")
    config["pg_port"] = load_envvar("PG_DB_PORT")
    config["pg_host"] = load_envvar("PG_DB_HOST")

    return config
