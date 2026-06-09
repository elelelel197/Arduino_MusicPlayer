STATE btnOutput(){
    bool onePulseBtnPlay = 0; // variable for debouncing the play button
    bool onePulseBtnPause = 0; // variable for debouncing the pause button
    bool onePulseBtnRestart = 0; // variable for debouncing the restart button
    bool onePulseBtnRead = 0; // variable for debouncing the read button
    bool onePulseBtnSetTrack = 0; // variable for debouncing the set track

    static bool lastBtnPlay = 0; // variable to store the last state of the play button
    static bool lastBtnPause = 0;
    static bool lastBtnRestart = 0;
    static bool lastBtnRead = 0;
    static bool lastBtnSetTrack = 0;

    static uint8_t btnPlayBuffer = 0; // buffer variable for the play button state
    static uint8_t btnPauseBuffer = 0; // buffer variable for the pause button state
    static uint8_t btnRestartBuffer = 0; // buffer variable for the restart button state
    static uint8_t btnReadBuffer = 0; // buffer variable for the read button state
    static uint8_t btnSetTrackBuffer = 0; // buffer variable for the set track button state

    onePulseBtnPlay = onepulse(debounce(digitalRead(btnPlayPin), btnPlayBuffer, lastBtnPlay));
    onePulseBtnPause = onepulse(debounce(digitalRead(btnPausePin), btnPauseBuffer, lastBtnPause));
    onePulseBtnRestart = onepulse(debounce(digitalRead(btnRestartPin), btnRestartBuffer, lastBtnRestart));
    onePulseBtnRead = onepulse(debounce(digitalRead(btnReadPin), btnReadBuffer, lastBtnRead));
    onePulseBtnSetTrack = onepulse(debounce(digitalRead(btnSetTrackPin), btnSetTrackBuffer, lastBtnSetTrack));

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