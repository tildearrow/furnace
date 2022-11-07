package org.tildearrow.furnace;

import android.content.Intent;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
  static final int TA_FILE_REQUEST=1000;

  public boolean showFileDialog() {
    Intent picker=new Intent(Intent.ACTION_GET_CONTENT);
    picker.setType("*/*");
    picker=Intent.createChooser(picker,"test");
    startActivityForResult(picker,TA_FILE_REQUEST);

    return true;
  }

  @Override protected void onActivityResult(int request, int result, Intent intent) {
    super.onActivityResult(request,result,intent);
    if (request==TA_FILE_REQUEST) {
      // TODO: fire an event here
    }
  }
}
