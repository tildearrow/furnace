package org.tildearrow.furnace;

import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
  static final int TA_FILE_REQUEST=1000;

  public void showFileDialog() {
    Intent picker=new Intent(Intent.ACTION_GET_CONTENT);
    picker.setType("*/*");
    picker=Intent.createChooser(picker,"test");
    startActivityForResult(picker,TA_FILE_REQUEST);
  }

  @Override protected void onActivityResult(int request, int result, Intent intent) {
    super.onActivityResult(request,result,intent);
    if (request==TA_FILE_REQUEST) {
      if (result==RESULT_OK) {
        Context context=getApplicationContext();
        Toast toast=Toast.makeText(context,"Got a file",Toast.LENGTH_SHORT);
        toast.show();
      }
    }
  }
}
