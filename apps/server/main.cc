#include "crow/crow_all.h"

#include <iostream>

int main(){
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([]() {
        return "Hello world!";
    });

    app.port(8181).run();
    return 0;
}