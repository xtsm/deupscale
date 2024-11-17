image = document.getElementById("image");
canvas = document.getElementById("canvas");

image.addEventListener("change", async (evt) => {
  if (image.files.length < 1) {
    console.error("no files selected");
    return;
  }
  var bytes = await image.files[0].bytes();
  var inFile = "/input." + image.files[0].name.split(".").pop();
  var outFile = "/output.png";
  FS.writeFile(inFile, bytes);
  var inFileEm = stringToNewUTF8(inFile);
  var outFileEm = stringToNewUTF8(outFile);
  var result = _DeupscaleFile(inFileEm, outFileEm);
  _free(inFileEm);
  _free(outFileEm);
  if (!result) {
    console.error("failed to process file");
    return;
  }
  var resultImage = await createImageBitmap(new Blob([FS.readFile(outFile)]));
  var ctx = canvas.getContext("bitmaprenderer");
  ctx.transferFromImageBitmap(resultImage);
})
