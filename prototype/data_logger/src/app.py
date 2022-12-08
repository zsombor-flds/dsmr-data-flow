import os
import csv
import json
from datetime import datetime

import pandas as pd
from flask import Flask, request
from sqlalchemy import create_engine

from get_config import read_config

app = Flask(__name__)
config = read_config()

OUT_FILE = "data.csv"

def db_connection(
    username: str, password: str, host: str, port: int, database: str
) -> "sqlalchemy.engine.base.Engine":
    """
    Init db connection
    """
    return create_engine(f"postgresql://{username}:{password}@{host}:{port}/{database}")


def load(
    engine: "sqlalchemy.engine.base.Engine",
    df: pd.DataFrame,
    table_name: str,
    schema: str,
) -> None:
    """
    Insert df to sql table
    """
    conn = engine.connect()
    # conn.execute(f"TRUNCATE TABLE {schema}.{table_name}")
    df.to_sql(
        name=table_name,
        con=engine,
        schema=schema,
        if_exists="append",
        index=False,
        index_label=None,
    )


def transform(data: dict) -> dict:
    timestamp_format = "%y%m%d%H%M%SW"
    data["power_timestamp"] = datetime.strptime(
        data["power_timestamp"], timestamp_format
    )
    return data


def persist_to_csv(data) -> None:
    if_exists = os.path.isfile(OUT_FILE)
    with open(OUT_FILE, "a", newline="") as csvfile:
        writer = csv.writer(csvfile)
        if not if_exists:
            writer.writerow(data.keys())

        writer.writerow(data.values())


@app.route("/sensor", methods=["POST"])
def handle_request():
    # print(request.json)
    raw_data = json.loads(request.data)
    final_data = transform(data=raw_data)
    persist_to_csv(data=final_data)

    df = pd.DataFrame([final_data])

    engine = db_connection(
        username=config["pg_username"],
        password=config["pg_password"],
        host=config["pg_host"],
        port=config["pg_port"],
        database=config["pg_db"],
    )
    load(engine=engine, df=df, table_name="test", schema="public")
    return "ok"


if __name__ == "__main__":
    app.run(host="0.0.0.0")
