# **Транспортный каталог**

Проект траспортного каталога, который позволяет создать базу данных общественного транспорта, визуализировать маршруты в формате SVG и найти кратчайший путь между остановками.

В проекте используются следующие технологии:
- JSON - формат взаимодействия с сервером
- Protobuf - хранение данных БД в сериализованном виде

## **Использование**
- Запрос на добавление данных в БД
  ```json
  "base_requests": [
    // добавление информации о маршруте
    {
      "type": "Bus",
      "name": "134",
      "stops": ["Stop-1", "Stop-2"],
      "is_roundtrip": false
    },
    // добавление информации об остановках
    {
      "type": "Stop",
      "name": "Stop-1",
      "latitude": 34.325467,
      "longitude": 45.351613,
      "road_distances": {"Stop-2": 1500}
    },
    {
      "type": "Stop",
      "name": "Stop-2",
      "latitude": 36.325467,
      "longitude": 45.121613,
      "road_distances": {"Stop-1": 1500}
    }
  ]
  ```
- Запрос на получение данных из БД
  ```json
  "stat_requests": [
    // получение карты в SVG формате
    { "id": 1, "type": "Map" },
    // получение информации об остановке
    { "id": 2, "type": "Stop", "name": "Stop-1" },
      // получение информации о маршруте
    { "id": 3, "type": "Bus", "name": "134" }
  ]
  ```

- Ответ БД на запросы
  ```json
  [
    // карта в SVG формате
      {
          "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n<polyline points=\"46.1,170 30,30 46.1,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n<text x=\"46.1\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">134</text>\n<text x=\"46.1\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">134</text>\n<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">134</text>\n<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">134</text>\n<circle cx=\"46.1\" cy=\"170\" r=\"5\" fill=\"white\"/>\n<circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n<text x=\"46.1\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Stop-1</text>\n<text x=\"46.1\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Stop-1</text>\n<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Stop-2</text>\n<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Stop-2</text>\n</svg>",
          "request_id": 1
      },
      // Информация об остановке Stop-1
      {
          "buses": [
              "134"
          ],
          "request_id": 2
      },
      // Информация о маршруте 134
      {
          "curvature": 0.00671543,
          "request_id": 3,
          "route_length": 3000,
          "stop_count": 3,
          "unique_stop_count": 2
      }
  ]
  ```
- Изображение карты

  <img src="./transport-catalogue/data/map.svg" height=200px>


## **Зависимости**

1. [С++17](https://en.cppreference.com/w/cpp/17)
2. [GCC(MinGW-w64)](https://www.mingw-w64.org/) 11+ version requires
3. [CMake](https://cmake.org) 3.8 version requires
4. [Protobuf](https://protobuf.dev/) use proto3