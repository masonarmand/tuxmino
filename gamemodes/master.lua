--[
--
-- File: master.lua
-- ---------------
-- Author: Mason Armand
-- Contributors:
--
--]

grandMasterReqs = { lvl300 = false, lvl500 = false, lvl999 = false }


--[ This Function gets called once upon selecting the game mode --]
function start()
    -- Initialization and backgrounds
    initGameMode("Master 1", 255, 255, 255) -- Name and frame color (r, g, b)
    initBackgrounds(10) -- Number of backgrounds
    for i = 0, 9, 1 do
        initBgIdx(i, "res/bg/gradeMaster1/0"..i..".png")
    end
    setBackground(0)
    setPieceTint(150, 150, 150, 255)
    setPlayfieldColor(5, 0, 10, 220)

    setDAS(16)
    setARR(1)
    setARE(30)
    setLineARE(0)
    setLockDelay(30)
    setClearSpeed(41)

    setGravity(4)
    setPreview(1)
    setBagRetry(4)
    setGradeType(0)
    setGrade(0)
    setScore(0)
    setSevenBag(false)
    setHold(false)
    setSonicDrop(true)
    setExtraKicks(false)
    set3D(false)
    setDrawNextBg(false)
    setBig(false)
    setInvis(false)
    setDisplayGrade(true)
    setGhost(true)
    setCreditRoll(false)
    setClearOnCredits(false)
    setMaxLevel(900)
    setSectionLevel(100)
    setLevel(0)
    setNextScore(400)
end

function reset()
    grandMasterReqs["lvl300"] = false
    grandMasterReqs["lvl500"] = false
    grandMasterReqs["lvl999"] = false
end

--[ This Function gets called every frame --]
function update()
end

--[ This Function gets called every time a piece lands --]
function advanceLevel(amount, lineCount)
    -- advance getLevel() if not on a level stop
    if ((getLevel() % 100 ~= 99 and getLevel() ~= 998) or lineCount ~= 0) then
        setLevel(amount + getLevel() + lineCount)
    end

    -- Calculate Score
    if (lineCount > 0) then
        combo = (lineCount - 1) * 2
        addScore((math.ceil((getLevel() + lineCount) / 4) + getSoftFrames()) * lineCount * combo) -- TODO combo and bravo
    end
    setSoftFrames(0)

    if (getLevel() >= 900) then
        setSectionLevel(999)
    else
        setSectionLevel(math.floor(getLevel() / 100 + 1) * 100)
    end


    if (getElapsedTime() <= 255 and getLevel() >= 300) then
       grandMasterReqs["lvl300"] = true
    end

    if (getElapsedTime() <= 450 and getLevel() >= 500) then
       grandMasterReqs["lvl500"] = true
    end

    if (getElapsedTime() <= 810 and getLevel() >= 999) then
       grandMasterReqs["lvl999"] = true
    end

    -- Set gravity based on getLevel()
	if     (getLevel() < 30)  then setGravity(4)
	elseif (getLevel() < 35)  then setGravity(6)
	elseif (getLevel() < 40)  then setGravity(8)
	elseif (getLevel() < 50)  then setGravity(10)
	elseif (getLevel() < 60)  then setGravity(12)
	elseif (getLevel() < 70)  then setGravity(16)
	elseif (getLevel() < 80)  then setGravity(32)
	elseif (getLevel() < 90)  then setGravity(48)
	elseif (getLevel() < 100) then setGravity(64)
	elseif (getLevel() < 120) then setGravity(80)
	elseif (getLevel() < 140) then setGravity(96)
	elseif (getLevel() < 160) then setGravity(112)
	elseif (getLevel() < 170) then setGravity(128)
	elseif (getLevel() < 200) then setGravity(144)
	elseif (getLevel() < 220) then setGravity(4)
	elseif (getLevel() < 230) then setGravity(32)
	elseif (getLevel() < 233) then setGravity(64)
	elseif (getLevel() < 236) then setGravity(96)
	elseif (getLevel() < 239) then setGravity(128)
	elseif (getLevel() < 243) then setGravity(160)
	elseif (getLevel() < 247) then setGravity(192)
	elseif (getLevel() < 251) then setGravity(224)
	elseif (getLevel() < 300) then setGravity(256)
	elseif (getLevel() < 330) then setGravity(512)
	elseif (getLevel() < 360) then setGravity(768)
	elseif (getLevel() < 400) then setGravity(1024)
	elseif (getLevel() < 420) then setGravity(1280)
	elseif (getLevel() < 450) then setGravity(1024)
	elseif (getLevel() < 500) then setGravity(768)
	else setGravity(5120)
    end

    -- Set next grade
    if     getScore() <	  400 then setGrade(0);   setNextScore(400)
	elseif getScore() <	  800 then setGrade(1);   setNextScore(800)
	elseif getScore() <   1400 then setGrade(2);  setNextScore(1400)
	elseif getScore() <   2000 then setGrade(3);  setNextScore(2000)
	elseif getScore() <   3500 then setGrade(4);  setNextScore(3500)
	elseif getScore() <   5500 then setGrade(5);  setNextScore(5500)
	elseif getScore() <   8000 then setGrade(6);  setNextScore(8000)
	elseif getScore() <  12000 then setGrade(7);  setNextScore(12000)
	elseif getScore() <  16000 then setGrade(8);  setNextScore(16000)
	elseif getScore() <  22000 then setGrade(9);  setNextScore(22000)
	elseif getScore() <  30000 then setGrade(10); setNextScore(30000)
	elseif getScore() <  40000 then setGrade(11); setNextScore(40000)
	elseif getScore() <  52000 then setGrade(12); setNextScore(52000)
	elseif getScore() <  66000 then setGrade(13); setNextScore(66000)
	elseif getScore() <  82000 then setGrade(14); setNextScore(82000)
	elseif getScore() < 100000 then setGrade(15); setNextScore(100000)
	elseif getScore() < 120000 then setGrade(16); setNextScore(120000)
	else setGrade(17)
    end

    if grandMasterReqs["lvl300"] == true and
       grandMasterReqs["lvl500"] == true and
       grandMasterReqs["lvl999"] == true and
       getScore() >= 126000              and
       getLevel() >= 999
    then
        setGrade(18) --Grand Master
    end

    if (getLevel() >= 999) then
        setLevel(999)
        setLevel(999)
        setCreditRoll(true)
    end

    if (getSectionLevel() ~= 999) then
        bgIdx = 0

        if (getSectionLevel() > 100 and getSectionLevel() ~= 999) then
            bgIdx = (getSectionLevel() - 100) / 100
        end

        setBackground(bgIdx)
    else
        setBackground(9)
    end
end
