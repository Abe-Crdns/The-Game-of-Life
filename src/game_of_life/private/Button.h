#ifndef _BUTTON_H
#define _BUTTON_H

#include "../GameGlobals.h"

#define BUTTON_WIDTH      65
#define BUTTON_HEIGHT     25
#define BUTTON_TEXT_SIZE  20

class Button {
  private:
    Coords position;
    std::string text;
    unsigned trim_text_size;
    bool is_clicked;

  public:
    Button(Coords pos, std::string btnText, unsigned trimTextSize = 0);

    Coords getPosition() const;
    std::string getText() const;
    bool getIsClicked() const;

    void click();
    void enable();
    void disable();
};

#endif