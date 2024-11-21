image = document.getElementById("image");
imageLabel = document.getElementById("imageLabel");
canvas = document.getElementById("canvas");

Module().then(module => {
  image.addEventListener("change", async (evt) => {
    if (image.files.length < 1) {
      console.error("no files selected");
      return;
    }
    var bytes = await image.files[0].bytes();
    var inFile = "/input." + image.files[0].name.split(".").pop();
    var outFile = "/output.png";

    module.FS.writeFile(inFile, bytes);
    var inFileEm = module.allocateUTF8(inFile);
    var outFileEm = module.allocateUTF8(outFile);
    var result = module._DeupscaleFile(inFileEm, outFileEm);
    module._free(inFileEm);
    module._free(outFileEm);
    module.FS.unlink(inFile);

    if (!result) {
      console.error("failed to process file");
      module.FS.unlink(outFile);
      return;
    }

    var resultImage = await createImageBitmap(new Blob([module.FS.readFile(outFile)]));
    module.FS.unlink(outFile);
    var ctx = canvas.getContext("bitmaprenderer");
    ctx.transferFromImageBitmap(resultImage);
  });
  image.disabled = false;
  imageLabel.textContent = "Select image file:"
});
