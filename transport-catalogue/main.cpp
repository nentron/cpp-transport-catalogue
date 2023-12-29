#include "request_handler.h"
#include "json_reader.h"

#include <iostream>


int main(){
    json_reader::JSONReader rd;
    rd.Read(std::cin );
    map_render::RenderSVG render(rd.GetRenderSettings());
    request_handler::RequestHandler handler{rd.GetDB(), std::move(render)};
    handler.ManageRequests(std::cout, rd.GetDocument().GetRoot().AsMap());
}