/*
 *  Author: Abraham Cardenas / https://github.com/Abe-Crdns
 *  TODO: - figure out how to center the button text 
 */

#ifndef _BUTTON_CPP
#define _BUTTON_CPP

#include <string>
#include "Button.h"

Button::Button(Coords pos, std::string btnText, unsigned trimTextSize){
  text = btnText;
  trim_text_size = trimTextSize;
  position = pos;
  is_clicked = false;

  GAME_WINDOW.DrawRectangle(
    pos.x, pos.y, 
    BUTTON_WIDTH, BUTTON_HEIGHT, 
    GREY, true
  );

  if(trimTextSize > 0)
    btnText = btnText.substr(trimTextSize);

  GAME_WINDOW.DrawString(
    btnText, 
    pos.x + 16, 
    pos.y + 3, 
    BLACK, 
    BUTTON_TEXT_SIZE
  );
}

Coords Button::getPosition() const{
  return position;
}

std::string Button::getText() const{
  return text;
}

bool Button::getIsClicked() const{
  return is_clicked;
}

void Button::click(){
  is_clicked = !is_clicked;

  if(is_clicked){
    GAME_WINDOW.DrawRectangle(
      position.x, position.y, 
      BUTTON_WIDTH, BUTTON_HEIGHT, 
      YELLOW, true
    );
  }
  else{
    GAME_WINDOW.DrawRectangle(
      position.x, position.y, 
      BUTTON_WIDTH, BUTTON_HEIGHT, 
      GREY, true
    );
  }

  std::string btnText = text;
  if(trim_text_size > 0)
    btnText = btnText.substr(3);

  GAME_WINDOW.DrawString(
    btnText, 
    position.x + 16, 
    position.y + 3, 
    BLACK, BUTTON_TEXT_SIZE
  );
}

void Button::enable(){
  GAME_WINDOW.DrawRectangle(
    position.x, position.y, 
    BUTTON_WIDTH, BUTTON_HEIGHT, 
    GREY, true
  );

  std::string btnText = text;
  if(trim_text_size > 0)
    btnText = btnText.substr(3);

  GAME_WINDOW.DrawString(
    btnText, 
    position.x + 16, 
    position.y + 3, 
    BLACK, BUTTON_TEXT_SIZE
  );
}

void Button::disable(){
  GAME_WINDOW.DrawRectangle(
    position.x, position.y, 
    BUTTON_WIDTH, BUTTON_HEIGHT, 
    DARK_GREY, true
  );

  std::string btnText = text;
  if(trim_text_size > 0)
    btnText = btnText.substr(3);

  GAME_WINDOW.DrawString(
    btnText, 
    position.x + 16, 
    position.y + 3, 
    BLACK, BUTTON_TEXT_SIZE
  );
}



#endif