#include "request_handler.h"
#include "json_reader.h"
#include "transport_router.h"

#include <iostream>


int main(){
    json_reader::JSONReader rd;
    rd.Read(std::cin);
    const map_render::RenderSVG render(rd.GetRenderSettings());
    const transport_directory::TransportCatalogue db = rd.GetDB();
    RouterHelper helper{rd.GetRoutingSettings(), db.GetAllStops().size()};
    helper.LoadGraph(db);
    request_handler::RequestHandler handler{db, render, helper};
    rd.ManageRequests(std::cout, handler);
}