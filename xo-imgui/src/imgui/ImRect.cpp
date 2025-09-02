/* ImRect.cpp */

#include "ImRect.hpp"

void
ImRect::draw_filled_rect_with_label(const char * text,
                                    const char * tooltip,
                                    const ImRect & rect,
                                    ImU32 fillcolor,
                                    ImU32 textcolor,
                                    ImDrawList * draw_list)
{
    draw_list->AddRectFilled(rect.top_left(),
                             rect.bottom_right(),
                             fillcolor);

    if ((rect.width() > 0.0) && (rect.height() > 0.0)) {
        ImGui::SetCursorScreenPos(rect.top_left());
        ImGui::InvisibleButton("ttbutton", ImVec2(rect.width(), rect.height()));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }
    }

    if (text) {
        auto textz = ImGui::CalcTextSize(text);

        /* N1 can be empty: but in that case don't bother to label it */
        if (textz.x < rect.width()) {
            draw_list->AddText(ImVec2(rect.x_mid() - 0.5 * textz.x,
                                      rect.y_mid() - 0.5 * textz.y),
                               textcolor,
                               text);
        }
    }
}

void
ImRect::draw_filled_rect(const char * tooltip,
                         const ImRect & rect,
                         ImU32 fillcolor,
                         ImDrawList * draw_list)
{
    draw_filled_rect_with_label(nullptr,
                                tooltip,
                                rect,
                                fillcolor,
                                IM_COL32(255, 255, 255, 255),
                                draw_list);
}



/* ImRect.cpp */
