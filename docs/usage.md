## Usage in detail:

When starting SmillaEnlarger you will see a husky named Smilla in the tabbed view on the right side, and a thumbnail version in the column to the left, together with some buttons and input fields.

To replace Smilla and open another input image,

- grab an image somewhere and drop it onto the enlarger window,
- r paste something from the clipboard,
- r open a file using &#39; **Open...**&#39; in the file menu.

Now you should see your image in the right view and a **thumbnail** of it on the left. The thumbnail shows a small version of the output image, giving you an impression of its contents and format.

### Cropping

In the right tab, titled &#39; **Cropping**&#39;, you can select a region of your image for enlarging. When you hold down the mouse within this view and drag it around, you will see that a frame appears, the **cropping rectangle**. It won&#39;t disappear when you release the mouse. You can **move** it by grabbing its interior or **resize** it by grabbing the frame. To make it disappear, click anywhere outside the rectangle.

The **contents** of this **cropping rectangle** are enlarged when you click &#39; **Enlarge&amp;Save**&#39;. If no rectangle is defined, the complete input image is enlarged.

In the combo box under the picture you can select one of several fixed **cropping formats** for the rectangle, for example &#39; **square**&#39;, or  &#39;**1 : sqrt(2)**&#39; ( the format of a normal Din A4 paper ). Use &#39; **free**&#39; to allow arbitrary cropping.

After clicking &#39; **Center View**&#39; the view will center around the selected region. This is useful for fine adjustments if you have marked a very small part of the input.

### Specifying the Output Dimensions

The size of the result is displayed under the **Thumbnail Preview**. To change it, you can use one of several methods listed in the **combo box** directly under the header &#39;Output Dimensions&#39; on the left.

The methods are:

- **Specify zoom factor** - directly set a percentage for the enlarged dimensions
- **Specify width of result** - give desired number of pixels for the width of the output image
- **Specify height of result** - give desired number of pixels for the height of the output image
- **Fit inside boundary** - set maximal pixel values for width and height of the result. The Enlarger will choose the maximal dimensions fitting into this boundary.
- **Stretch to fit** - set the pixel values for width and height of the result. The Enlarger will stretch the image to fit into this rectangle, possibly changing the aspect ratio.
- **Crop to fit** - set the pixel values for width and height of the result. The image will be enlarged to cover the given rectangle, the overlapping parts are cut away. ( You can additionally change the aspect ratio by changing &#39;StretchX&#39; )
- **Fit inside, add bars** - set the pixel values for width and height of the result. The image will be enlarged to fit inside the given rectangle, which then is filled up with black margins.

For each of these methods you can get an impression how it works by changing the size and format of the **cropping rectangle** and watching the effects and displayed size in the **Thumbnail Preview**. Some methods are mostly useful for **batch processing.**

### Drop, Paste, Open

SmillaEnlarger supports **Drag &amp; Drop** : You can drag a file or image from any folder or from another program offering Drag&amp;Drop and drop it onto the enlarger window. The enlarger then tries to open it as new source.

If you drop a **folder** or multiple images at once, **batch processing** is started instead of displaying a new source:  The images are **directly** moved to the **job queue** using the current enlarger settings.

You also can paste the contents of the clipboard, i.e. use **Copy** within another program on a file or image and then in SmillaEnlarger call **Paste** from the File Menu ( or press **Ctrl V** / **Cmd V** ) to load it into the enlarger.

Or choose &#39; **Open...**&#39; in the file menu or press **Ctrl-O** / **Cmd-O**. Or click &#39; **Open...**&#39; above the view in the **&#39;Cropping&#39;** tab.

After selecting a picture, you will see it displayed in the &#39;Cropping&#39; tab. To quickly switch to another file in the same folder, click onto the box containing the filename, you will see that it&#39;s a combo-box listing all pictures in the source folder. Supported types for loading are


 **JPG** , **GIF** , **TIF** , **BMP** , **PPM** and **PNG**.



### Enlarge &amp; Save

To **save** the enlarged version of the **selected area** in the **&#39;Cropping&#39; view** , press &#39; **Enlarge &amp; Save**&#39;. Calculating the magnified picture will take some time, when the progress bar reaches 100%, the result is saved.

Under the header **Write Result to:**&#39; on the bottom left, you can see where the result will be saved. The enlarger automatically chooses the source name and type with an appended \_e and a number as the result&#39;s filename, but you can enter something else as long as the ending belongs to one of the supported image types. The appended number is increased if you enlarge several parts of the same source. By default the enlarged picture is written into the source folder, uncheck **&#39;Use Source Folder&#39;** and click **&#39;Change Folder&#39;** to choose something different. Supported types for saving are

 **JPG** , **TIF** , **BMP** , **PPM** and **PNG**.

You can set your preferred output format and ( for **JPEG** images ) the output **quality** in the Preferences dialog.

The new job is pushed into the **Job Queue** and processed in the background, you can directly continue working with the enlarger.

Changes of enlarger settings **don&#39;t have any influence on existing jobs**.
The settings of the job ( size, destination, parameters ) are those present at the moment of pressing &#39; **Enlarge &amp; Save**&#39;, they can not be changed afterwards.

### The Parameter Tab

On the right side you can **choose a tab** titled &#39; **Parameter**&#39;. There you can **preview** a part of the output image with the desired zoom. This view shows **only a part** of the result. It is only for giving you an impression of the zoom and for testing enlarging parameters.

You can **drag around the contents** with the mouse. When you click somewhere in the little &#39; **Thumbnail Preview**&#39; on the left, the region around this point is displayed magnified in the parameter view.

By clicking onto the **Preview** button, you can see what the enlarger does with the contents of the magnified view.

In the Parameter Tab you can adjust some enlargement parameters:

- **Sharpness:** higher values lead to sharper edges, might look artificial if too sharp
- **Flatness:** higher values produce more &#39;painted&#39; looking results with less gradients
- **PreSharpen:** applies simple sharpening to the source before enlarging
- **Dithering:** add a slightly analogue looking grainy structure to the result
- **DeNoise:** remove some noise and artifacts from the source
- **FractNoise:** just a gimmick: get some irregularity into your result; contours and colors are modified by plasma fractal noise.

You can switch between predefined settings in the **combo box**.
To change a setting, first check &#39; **Allow Changes**&#39;.
You can create a new set cloning the current one by clicking &#39; **New**&#39;. For a new settings name type something into the combo box, don&#39;t forget to hit the &#39; **&#39;Return**&#39; key at the end to adopt the changed name.

You can test the effects of different settings by clicking onto **Preview.**



**Batch Processing**

SmillaEnlarger can automatically process a batch of images with the same settings:

If you **drop a folder** onto the enlarger, the contents are scanned for images of supported type ( subfolders are ignored ). Onto those images the current parameters and the **resizing method** chosen in the **Output Dimensions** boxare applied **at once**. The new jobs are moved to the queue **automatically** , you don&#39;t have to press &#39;Enlarge&amp;Save&#39;.

The results are put into a **new folder** , the name is that of the source with an appended &#39;\_e&#39;.

Also, when you drop multiple files, those are likewise pushed into the queue automatically, but in this case no new folder is created for the batch results.

**Important:** Before dropping more than one file, check if all settings are as you want, especially look under &#39; **Write Result to:**&#39; in which **folder** the results will be saved ( probably you will want to uncheck &#39;Use Source Folder&#39; and give a new location where your batch results are saved together ).



### The Job Queue

SmillaEnlarger makes use of a **Job Queue**. New calculations are appended to a list of jobs, the entries of the list are then by and by given to the calculation threads in the background, while you can continue your work at once. The progress of the job queue is shown by the progress bar at the bottom left of the window . For detailed information about the jobs in the queue, open the **Jobs Tab**.

In this tab you can **clear** the complete queue, discarding all running and waiting calculations.
Or select one job and click &#39; **remove job**&#39; to kill it.
With &#39; **clean up**&#39; you remove all finished jobs from the list.

**SmillaEnlarger as a Command Line Tool**

SmillaEnlarger can also be used from the command line.

**Windows** users have to call **SmillaEnlargerCL.exe** instead of SmillaEnlarger.exe .

**Mac** users have to use the binary deep inside the SmillaEnlarger.app bundle. To create a **symbolic link** to the binary, open a terminal window, change to the directory you want to put your link and call

ln -s /Path/to/SmillaEnlarger.app/Contents/MacOS/SmillaEnlarger

with the appropriate path to your SmillaEnlarger.
To prevent lengthy path-typing, simply write   ln -s  into the terminal, then go to your SmillaEnlarger icon, **option-click** onto it and open the application bundle. 
Go into the subfolder Contents/MacOS/, inside you will find the SmillaEnlarger binary, grab it with the mouse and drop it onto the terminal window, the terminal then will fill in the full path to the binary.


 ### Usage of SmillaEnlarger

SmillaEnlarger [&lt; sourcename &gt;] [-options...]

with options

-z &lt;number&gt; / -zoom &lt;number&gt; 
Set zoom-factor to &lt;number&gt; percent ( integer value ).

-o &lt;filename&gt;
Write result to file &lt;filename&gt; .

-saveto &lt;foldername&gt;
Write results into folder &lt;foldername&gt; .


**Output Dimensions:** 

-width &lt;sizex&gt; and -height &lt;sizey&gt; 
set size of resulting image.
If both width and height are given, aspect ratio is changed by default. Additionally, if you have set -width AND -height , you can set one of the following options: 

-fit
Fit output inside the given rectangle.

-fitandbars 
Fit output inside the given rectangle,
fill up with black margins.

-cover 
Cover the given rectangle.

-coverandcrop 
Cover the given rectangle, cut away the overlapping parts.


**Enlarge Parameters:** 

-sharp &lt; n &gt;, -flat &lt; n &gt;, -dither &lt; n &gt; 
-deNoise &lt; n &gt;, -preSharp &lt; n &gt;, -fNoise &lt; n &gt; 
Set the enlarge parameters with integer numbers &lt; n &gt; between 0 and 100.


-quality &lt;number&gt; 
Set image quality of the result.

-h / -help
Print this help.

-i 
Start in interactive mode.

**Original (old) Project page:**    [http://sourceforge.net/projects/imageenlarger/](http://sourceforge.net/projects/imageenlarger/)

**Project page:**    [https://github.com/lupoDharkael/smilla-enlarger](https://github.com/lupoDharkael/smilla-enlarger)



### License / Copyright

Copyright (C) 2009 Mischa Lusteck

Copyright (C) 2017 Alejandro Sirgo

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License ( license.rtf ) for more details.

You should have received a copy of the GNU General Public License in license.rtf along with this program; if not, see &lt;http://www.gnu.org/licenses/&gt;.

 
