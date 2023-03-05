--[
--
-- File: tutorial.lua
-- ---------------
-- Author: Mason Armand
-- Contributors:
--
--]

--[[ This Function gets called once when the game is loaded --]]
function start()
    -- Set the name of our game mode to "Tutorial" with a red frame
    initGameMode("Tutorial", 255, 0, 0)
    -- Set the number of backgrounds to two
    initBackgrounds(2)

    -- Load our two backgrounds
    -- initBgIdx(0, "yourbackground.png")
    -- initBgIdx(1, "anotherbackground.png")

    -- Set our background to the first background
    setBackground(0)

    -- game mode settings
    setDAS(16)             -- Delayed Auto Shift
    setARR(1)              -- Auto Repeate Rate
    setARE(30)             -- Appearance Delay
    setLineARE(0)          -- Line Appearance Delay
    setClearSpeed(41)      -- Line Clear Speed
    setLockDelay(40)       -- Lock delay
    setGravity(4)          -- Gravity (max is 5120, which is 20G)
    setPreview(3)          -- Number of next pieces displayed
    setBagRetry(6)         -- Number of times that will be attempted to generate a piece not in the bag
    setGradeType(0)        -- Choose between two predefined grade types (0, 1) or create your own (2)
    setGrade(0)            -- Grade (0-17) 0 = 9, 17 = Grand Master
    setSevenBag(true)      -- Use 7 bag piece generation?
    setHold(true)          -- Enable hold?
    setSonicDrop(true)     -- Enable sonic/hard drop?
    setExtraKicks(true)    -- Enable extra classic kicks? (extra I and T kicks)
    set3D(true)            -- Enable psuedo 3D effect?
    setDrawNextBg(true)    -- Enable background behind piece preview?
    setBig(false)          -- Enable big mode?
    setInvis(false)        -- Enable invisible pieces?
    setDisplayGrade(false) -- Display internal grade (based on gradeType)?
    setGhost(true)         -- Enable piece ghost?
    setCreditRoll(false)   -- Show credit roll?
    setMaxLevel(900)       -- Maximum select level
    setSectionLevel(100)   -- Section Level
    setLevel(0)            -- Current Level
end


--[ This Function gets called every frame --]
function update()
end

--[ This Function gets called every time a piece lands --]
function advanceLevel(amount, level, lineCount)
    setLevel(level + amount + lineCount)

    if (level >= 100) then
        setBackground(1)
        setSectionLevel(200)
        setGravity(5120)
    else
        setBackground(0)
        setSectionLevel(100)
        setGravity(4)
    end
end
