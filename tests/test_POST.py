from urllib.request import Request
import requests
import socket
import pytest
import os
from pathlib import Path
import subprocess
import random
import string
from functools import partial

from common import *

s = requests.Session()

def test_POST_on_not_authorized_route_with_size_0():
    response = requests.post(BASE_URL + "/no-post", data="foo")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_on_not_authorized_route_with_size_10():
    response = requests.post(BASE_URL + "/no-post", data=("x" * 10))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_on_not_authorized_route_with_size_1000():
    response = requests.post(BASE_URL + "/no-post", data=("x" * 1000))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_POST_without_content_length():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"POST / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent: curl/7.77.0\r\n".encode()) # No fieldname 
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 411 Length Required"

def test_POST_with_content_length_too_short_check_status():
    headers = {}
    req = requests.Request(method='POST', url=BASE_URL + "/scripts/echo.php", headers=headers)
    prepped = req.prepare()
    prepped.headers["Content-Length"] = 10
    prepped.body = b'Seriously, send exactly these bytes.'
    print(prepped.headers)
    response = s.send(prepped)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_content_length_too_short_check_value():
    headers = {}
    req = requests.Request(method='POST', url=BASE_URL + "/scripts/echo.php", headers=headers)
    prepped = req.prepare()
    prepped.headers["Content-Length"] = 10
    prepped.body = b'Seriously, send exactly these bytes.'
    print(prepped.headers)
    response = s.send(prepped)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.content.decode() == "Seriously,"

@pytest.mark.skip(reason="Not Implemented")
def test_POST_with_content_length_too_long():
    pass

def gen_chunked_1():
    yield b'hi'
    yield b'there'

def test_POST_with_transfer_encoding_chunked_1_check_status():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_1(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_transfer_encoding_chunked_1_check_value():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_1(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    desired_output = "".join([chunk.decode() for chunk in gen_chunked_1()])
    assert response.content.decode() == desired_output

def gen_chunked_2():
    for _ in range(8): # Cannot exceed 8388608 bytes for php-cgi so we send 8000000
        with open(WWW_DIR / 'onemillion.txt', "r") as f:
            yield f.read().encode()

def test_POST_with_transfer_encoding_chunked_2_check_status(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_2(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_with_transfer_encoding_chunked_2_check_value(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/scripts/echo.php", data=gen_chunked_2(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    desired_output = "".join([chunk.decode() for chunk in gen_chunked_2()])
    assert response.content.decode() == desired_output

def gen_chunked_3():
    yield b'lololololl'
    yield b'lalallalalalalalalalal'

def test_POST_with_transfer_encoding_chunked_body_limit_1():
    headers = {"Transfer-Encoding": "chunked"}
    response = requests.post(BASE_URL + "/post/limit", data=gen_chunked_3(), headers=headers)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 413

def test_POST_with_transfer_encoding_chunked_body_limit_2(create_1_million_file):
    headers = {"Transfer-Encoding": "chunked"}
    try:
        response = requests.post(BASE_URL + "/post/limit", data=gen_chunked_2(), headers=headers)
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_expect_413():
    try:
        response = requests.post(BASE_URL + "/post/limit", data=("a" * 1000))
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_expect_200():
    response = requests.post(BASE_URL + "/post/limit", data=("a" * 10))
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_POST_body_limit_1_million_expect_413(create_1_million_file):
    try:
        with open(WWW_DIR / 'onemillion.txt', "r") as f:
            response = requests.post(BASE_URL + "/post/limit", data=f.read())
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

def test_POST_body_limit_one_over_limit_expect_413():
    try:
        response = requests.post(BASE_URL + "/post/limit", data=("a" * 11))
        pretty_print_request(response.request)
        pretty_print_response(response)
        assert response.status_code == 413
    except requests.exceptions.ConnectionError: # Connection is closed by the server before the file has been fully sent
        pass
    except Exception:
        raise

@pytest.mark.parametrize("size", [10, 100, 1000, 10_000, 100_000, 1_000_000, 10_000_000, 100_000_000])
def test_POST_check_file_created_txt(size):
    parts = int(size / 10)
    filename = random_filename()
    payload = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(parts))
    response = requests.post(BASE_URL + f"/post/{filename}", data=(payload * 10))
    print("Total payload size %d", size)
    print("Filename %s", filename)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert len(response.content.decode()) == size
    assert response.content.decode() == payload * 10

@pytest.mark.parametrize("size", [10, 100, 1000, 10_000, 100_000, 1_000_000, 10_000_000, 100_000_000])
def test_POST_check_file_created_bin(size):
    parts = int(size / 10)
    filename = random_filename()
    payload = os.urandom(parts)
    response = requests.post(BASE_URL + f"/post/{filename}", data=(payload * 10))
    print("Total payload size %d", size)
    print("Filename %s", filename)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert len(response.content) == size
    assert response.content == payload * 10

@pytest.mark.parametrize("size", [10, 100, 1000, 10_000, 100_000, 1_000_000, 10_000_000, 100_000_000])
def test_POST_check_file_created_chunked_txt(size):
    n_chunks = 10
    chunk_size = int(size/10)
    chunk = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(chunk_size))
    def gen_chunked():
        for _ in range(n_chunks):
            yield chunk.encode()
    filename = random_filename()
    response = requests.post(BASE_URL + f"/post/{filename}", data=gen_chunked())
    print("Total payload size %d", size)
    print("Chunk size %d %d", chunk_size, len(chunk))
    print("Number of chunks %d", n_chunks)
    print("Filename %s", filename)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert len(response.content.decode()) == size
    assert response.content.decode() == chunk * 10

@pytest.mark.parametrize("size", [10, 100, 1000, 10_000, 100_000, 1_000_000, 10_000_000, 100_000_000])
def test_POST_check_file_created_chunked_bin(size):
    n_chunks = 10
    chunk_size = int(size/10)
    chunk = os.urandom(chunk_size)
    def gen_chunked():
        for _ in range(n_chunks):
            yield chunk
    filename = random_filename()
    response = requests.post(BASE_URL + f"/post/{filename}", data=gen_chunked())
    print("Total payload size %d", size)
    print("Chunk size %d %d", chunk_size, len(chunk))
    print("Number of chunks %d", n_chunks)
    print("Filename %s", filename)
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert len(response.content) == size
    assert response.content == chunk * 10

def test_POST_check_append_txt():
    payload_size = 1000
    payload = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(payload_size))
    filename = random_filename()
    for _ in range(10):
        response = requests.post(BASE_URL + f"/post/{filename}", data=payload)
        assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert response.content.decode() == payload * 10

def test_POST_check_append_bin():
    payload_size = 1000
    payload = os.urandom(payload_size)
    filename = random_filename()
    for _ in range(10):
        response = requests.post(BASE_URL + f"/post/{filename}", data=payload)
        assert response.status_code == 201
    response = requests.get(BASE_URL + f"/data/{filename}")
    assert response.status_code == 200
    assert response.content == payload * 10

def test_POST_form_data_single_file_txt(tmpdir):
    sub = tmpdir.mkdir("sub")
    file = sub.join("file.txt")
    content = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(10000))
    file.write(content)
    response = requests.post(BASE_URL + "/post/", files={"file": open(str(file), "r")})
    assert response.status_code == 200
    response = requests.get(BASE_URL + f"/data/{file.basename}")
    assert response.status_code == 200
    assert response.content.decode() == content

def test_POST_form_data_single_file_bin(tmpdir):
    sub = tmpdir.mkdir("sub")
    file = sub.join("file.txt")
    content = os.urandom(10000)
    file.write_binary(content)
    response = requests.post(BASE_URL + "/post/", files={"file": open(str(file), "rb")})
    assert response.status_code == 200
    response = requests.get(BASE_URL + f"/data/{file.basename}")
    assert response.status_code == 200
    assert response.content == content
    
def test_POST_form_data_multiple_files_txt(tmpdir):
    sub = tmpdir.mkdir("sub")
    file1 = sub.join("file1.txt")
    file2 = sub.join("file2.txt")
    file3 = sub.join("file3.txt")
    content = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(10000))
    file1.write(content)
    file2.write(content)
    file3.write(content)
    response = requests.post(BASE_URL + "/post/", files={"file1": open(str(file1), "r"), "file2": open(str(file2), "r"), "file3": open(str(file3), "r")})
    assert response.status_code == 200
    response = requests.get(BASE_URL + f"/data/{file1.basename}")
    assert response.status_code == 200
    assert response.content.decode() == content
    response = requests.get(BASE_URL + f"/data/{file2.basename}")
    assert response.status_code == 200
    assert response.content.decode() == content
    response = requests.get(BASE_URL + f"/data/{file3.basename}")
    assert response.status_code == 200
    assert response.content.decode() == content

def test_POST_form_data_multiple_files_bin(tmpdir):
    sub = tmpdir.mkdir("sub")
    file1 = sub.join("file1.txt")
    file2 = sub.join("file2.txt")
    file3 = sub.join("file3.txt")
    content = os.urandom(10000)
    file1.write_binary(content)
    file2.write_binary(content)
    file3.write_binary(content)
    response = requests.post(BASE_URL + "/post/", files={"file1": open(str(file1), "rb"), "file2": open(str(file2), "rb"), "file3": open(str(file3), "rb")})
    assert response.status_code == 200
    response = requests.get(BASE_URL + f"/data/{file1.basename}")
    assert response.status_code == 200
    assert response.content == content
    response = requests.get(BASE_URL + f"/data/{file2.basename}")
    assert response.status_code == 200
    assert response.content == content
    response = requests.get(BASE_URL + f"/data/{file3.basename}")
    assert response.status_code == 200
    assert response.content == content

