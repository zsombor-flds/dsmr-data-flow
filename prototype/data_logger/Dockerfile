FROM python:3.9.10

RUN apt-get update && \
    apt-get install -y libpq-dev gcc
COPY requirements.txt ./requirements.txt
RUN pip install -r requirements.txt
COPY ./src/ /src/
WORKDIR /src
ENV PYTHONPATH "/src"
CMD python3 app.py