#include "../visualizer/editor_scroll.h"
#include <assert.h>

int main(void) {
    /* Focused caret at line 1; wheel reaches line 31 and idle frames retain it. */
    unsigned top=editor_scroll(0,100,20,0,30,false);
    assert(top==30);
    for (int frame=0;frame<120;frame++) {
        top=editor_scroll(top,100,20,0,0,false);
        assert(top==30);
    }
    /* Navigation/editing reveals the caret; a click in view preserves scroll. */
    assert(editor_scroll(top,100,20,0,0,true)==0);
    assert(editor_scroll(top,100,20,35,0,true)==30);
    assert(editor_scroll(top,100,20,75,0,true)==56);
    assert(editor_scroll(0,100,20,0,-30,false)==0);
    assert(editor_scroll(90,100,20,0,30,false)==99);
    assert(editor_scroll(30,2,20,0,0,false)==1);
    assert(editor_scroll(0,0,0,0,0,true)==0);
    return 0;
}
