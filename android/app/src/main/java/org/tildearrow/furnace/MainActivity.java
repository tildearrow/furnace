package org.tildearrow.furnace;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
  static final int TA_FILE_REQUEST=1000;
  static final int TA_FILE_SAVE_REQUEST=1001;

  public void showFileDialog() {
    Intent picker=new Intent(Intent.ACTION_GET_CONTENT);
    picker.setType("*/*");
    picker=Intent.createChooser(picker,"test");
    startActivityForResult(picker,TA_FILE_REQUEST);
  }

  public void showSaveFileDialog() {
    Intent picker=new Intent(Intent.ACTION_CREATE_DOCUMENT);
    picker.addCategory(Intent.CATEGORY_OPENABLE);
    picker.setType("*/*");

    startActivityForResult(picker,TA_FILE_SAVE_REQUEST);
  }

  @Override protected void onActivityResult(int request, int result, Intent intent) {
    super.onActivityResult(request,result,intent);
    if (request==TA_FILE_REQUEST) {
      if (result==RESULT_OK) {
        Uri path=intent.getData();

        Context context=getApplicationContext();
        Toast toast=Toast.makeText(context,path.toString(),Toast.LENGTH_SHORT);
        toast.show();
      }
    }
  }
}
