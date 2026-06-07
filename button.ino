STATE btnOutput(){
    static bool onePulsebtnPlay = 0; // variable for debouncing the play button
    static bool onePulsebtnPause = 0; // variable for debouncing the pause button
    static bool onePulsebtnRestart = 0; // variable for debouncing the restart button
    static bool onePulsebtnRead = 0; // variable for debouncing the read button
    static bool onePulsebtnSetTrack = 0; // variable for debouncing the set track

    static uint8_t btnPlayBuffer = 0; // buffer variable for the play button state
    static uint8_t btnPauseBuffer = 0; // buffer variable for the pause button state
    static uint8_t btnRestartBuffer = 0; // buffer variable for the restart button state
    static uint8_t btnReadBuffer = 0; // buffer variable for the read button state
    static uint8_t btnSetTrackBuffer = 0; // buffer variable for the set track button state

    onePulsebtnPlay = onepulse(debounce(digitalRead(btnPlayPin), btnPlayBuffer));
    onePulsebtnPause = onepulse(debounce(digitalRead(btnPausePin), btnPauseBuffer));
    onePulsebtnRestart = onepulse(debounce(digitalRead(btnRestartPin), btnRestartBuffer));
    onePulsebtnRead = onepulse(debounce(digitalRead(btnReadPin), btnReadBuffer));
    onePulsebtnSetTrack = onepulse(debounce(digitalRead(btnSetTrackPin), btnSetTrackBuffer));

    if (onePulsebtnPlay) {
        return STATE_PLAY;
    } else if (onePulsebtnPause) {
        return STATE_PAUSED;
    } else if (onePulsebtnRestart) {
        return STATE_RESTART;
    } else if (onePulsebtnRead) {
        return STATE_READ;
    } else if (onePulsebtnSetTrack) {
        return STATE_SET_TRACK;
    } else {
        return STATE_IDLE;
    }
}