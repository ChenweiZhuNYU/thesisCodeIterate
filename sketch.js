let mSerial;
let connectButton;
let readyToReceive = true;
let lastRequestTime = 0;
let requestInterval = 100;

let scenes = [];
let buttonStates = [false, false];
let lastTrueTime = [0, 0];
let resetCooldownUntil = [0, 0];

let media = [
  { img: "chap1S.jpg", video: "chap1V.mp4" },
  { img: "chap2S.jpg", video: "chap2V.mp4" }
];

function preload() {
  for (let i = 0; i < media.length; i++) {
    let vid = createVideo(media[i].video);
    vid.hide();
    vid.elt.muted = true;
    scenes.push({
      img: loadImage(media[i].img),
      video: vid,
      playing: false
    });
  }
}

function setup() {
  createCanvas(windowWidth, windowHeight);
  mSerial = createSerial();
  connectButton = createButton("\ud83d\udd0c \u8fde\u63a5 Arduino");
  connectButton.position(20, 20);
  connectButton.mousePressed(connectToSerial);
}

function draw() {
  background(0);
  let sceneWidth = width / 2;
  let now = millis();

  for (let i = 0; i < 2; i++) {
    let x = i * sceneWidth;
    image(scenes[i].playing ? scenes[i].video : scenes[i].img, x, 0, sceneWidth, height);
  }

  // 每 100ms 发送请求
  if (mSerial.opened() && readyToReceive && now - lastRequestTime > requestInterval) {
    mSerial.clear();
    mSerial.write(0xAB);
    lastRequestTime = now;
    readyToReceive = false;
  }

  if (mSerial.availableBytes() > 0) {
    receiveSerial();
  }

  checkResetLogic();
}

function receiveSerial() {
  let line = mSerial.readUntil("\n").trim();
  if (!line) return;

  try {
    let json = JSON.parse(line);
    let data = json.data;

    for (let i = 0; i < 2; i++) {
      if (data[`button${i + 1}`]) {
        buttonStates[i] = true;
        if (lastTrueTime[i] === 0) {
          lastTrueTime[i] = millis();
        }
      } else {
        buttonStates[i] = false;
        lastTrueTime[i] = 0;
      }
    }

    if (buttonStates[0] && !scenes[0].playing) playScene(0);
    if (buttonStates[1] && !scenes[1].playing) playScene(1);

  } catch (e) {
    console.error("\u274c JSON \u89e3\u6790\u5931\u8d25:", e, line);
  }
  readyToReceive = true;
}

function checkResetLogic() {
  let now = millis();

  for (let i = 0; i < 2; i++) {
    if (
      buttonStates[i] &&
      lastTrueTime[i] > 0 &&
      now - lastTrueTime[i] > 5000 &&
      now > resetCooldownUntil[i]
    ) {
      if (i === 0) {
        // Reset all scenes
        for (let s of scenes) {
          s.playing = false;
          s.video.stop();
          s.video.hide();
        }
        console.log("\u267b\ufe0f Reset triggered by button 1");
      } else if (i === 1) {
        // Just log fallback trigger
        console.log("\u26a1 Backup: button 2 long-press fallback");
      }
      resetCooldownUntil[i] = now + 3000;
    }
  }
}

function playScene(index) {
  let scene = scenes[index];
  if (!scene.playing) {
    scene.playing = true;
    scene.video.time(0);
    scene.video.play();
    scene.video.onended(() => {
      scene.playing = false;
      scene.video.hide();
    });
  }
}

function connectToSerial() {
  if (!mSerial.opened()) {
    mSerial.open(9600);
    readyToReceive = true;
    connectButton.remove();
  }
}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}
