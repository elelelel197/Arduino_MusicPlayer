STATE btnOutput(){
    static bool lastDebBtnPlay = 0; // variable to store the last state of the play button for deb
    static bool lastDebBtnPause = 0;
    static bool lastDebBtnRestart = 0;
    static bool lastDebBtnRead = 0;
    static bool lastDebBtnSetTrack = 0;

    static bool lastOpBtnPlay = 0; // variable to store the last state of the play button for Op
    static bool lastOpBtnPause = 0;
    static bool lastOpBtnRestart = 0;
    static bool lastOpBtnRead = 0;
    static bool lastOpBtnSetTrack = 0;

    static uint8_t btnPlayBuffer = 0; // buffer variable for button states
    static uint8_t btnPauseBuffer = 0; 
    static uint8_t btnRestartBuffer = 0; 
    static uint8_t btnReadBuffer = 0; 
    static uint8_t btnSetTrackBuffer = 0; 

    bool onePulseBtnPlay = onepulse(debounce(digitalRead(btnPlayPin), btnPlayBuffer, lastDebBtnPlay), lastOpBtnPlay);
    bool onePulseBtnPause = onepulse(debounce(digitalRead(btnPausePin), btnPauseBuffer, lastDebBtnPause), lastOpBtnPause);
    bool onePulseBtnRestart = onepulse(debounce(digitalRead(btnRestartPin), btnRestartBuffer, lastDebBtnRestart), lastOpBtnRestart);
    bool onePulseBtnRead = onepulse(debounce(digitalRead(btnReadPin), btnReadBuffer, lastDebBtnRead), lastOpBtnRead);
    bool onePulseBtnSetTrack = onepulse(debounce(digitalRead(btnSetTrackPin), btnSetTrackBuffer, lastDebBtnSetTrack), lastOpBtnSetTrack);

    Serial.print("Button input:");
    Serial.print(onePulseBtnPlay);
    Serial.print(onePulseBtnPause);
    Serial.print(onePulseBtnRestart);
    Serial.print(onePulseBtnRead);
    Serial.println(onePulseBtnSetTrack);

    if (onePulseBtnPlay) {
        return STATE_PLAY;
    } else if (onePulseBtnPause) {
        return STATE_PAUSED;
    } else if (onePulseBtnRestart) {
        return STATE_RESTART;
    } else if (onePulseBtnRead) {
        return STATE_READ;
    } else if (onePulseBtnSetTrack) {
        return STATE_SET_TRACK;
    } else {
        return STATE_IDLE;
    }
}