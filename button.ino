STATE btnOutput(){
    static int debouncebtnPlay = 0; // variable for debouncing the play button
    static int debouncebtnPause = 0; // variable for debouncing the pause button
    static int debouncebtnRestart = 0; // variable for debouncing the restart button
    static int debouncebtnRead = 0; // variable for debouncing the read button
    static int debouncebtnSetTrack = 0; // variable for debouncing the set track

    static byte btnPlayBuffer = 0; // buffer variable for the play button state
    static byte btnPauseBuffer = 0; // buffer variable for the pause button state
    static byte btnRestartBuffer = 0; // buffer variable for the restart button state
    static byte btnReadBuffer = 0; // buffer variable for the read button state
    static byte btnSetTrackBuffer = 0; // buffer variable for the set track button state

    debouncebtnPlay = debounce(digitalRead(btnPlayPin), debouncebtnPlay);
    debouncebtnPause = debounce(digitalRead(btnPausePin), debouncebtnPause);
    debouncebtnRestart = debounce(digitalRead(btnRestartPin), debouncebtnRestart);
    debouncebtnRead = debounce(digitalRead(btnReadPin), debouncebtnRead);
    debouncebtnSetTrack = debounce(digitalRead(btnSetTrackPin), debouncebtnSetTrack); 

    if (debouncebtnPlay) {
        return PLAY;
    } else if (debouncebtnPause) {
        return PAUSED;
    } else if (debouncebtnRestart) {
        return RESTART;
    } else if (debouncebtnRead) {
        return READ;
    } else if (debouncebtnSetTrack) {
        return SET_TRACK;
    } else {
        return IDLE;
    }
}