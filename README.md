# Parsing JSON for HTTP client

This is just some experimentation with parsing JSON with [ESP HTTP Client](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_client.html).

It works with a callback function that is called whenever results are received. If the data is JSON and you immediately want to process it, you need a JSON parser that can process chunks of data. If you immediately want to process the parsed JSON, you could use some technique like [protothreads for C](http://dunkels.com/adam/pt/index.html).
