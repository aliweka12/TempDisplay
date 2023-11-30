/**
 * @file main.c
 * @brief Example program to demonstrate LCD display and temperature reading functionality.
 *
 * This program initializes the LCD display of a specific hardware setup and displays the room temperature.
 * It utilizes several peripherals including LTDC, DMA2D, and DSI. The temperature is simulated in this example.
 */

#include "main.h"
#include "image_320x240_argb8888.h"
#include <string.h>
#include <stdio.h>

// External handlers for LTDC and DSI peripherals
extern LTDC_HandleTypeDef hltdc_eval;
extern DSI_HandleTypeDef hdsi_eval;

// DMA2D handler
static DMA2D_HandleTypeDef hdma2d;

// Display configuration constants
#define VSYNC           1
#define VBP             1
#define VFP             1
#define VACT            480
#define HSYNC           1
#define HBP             1
#define HFP             1
#define HACT            800
#define LAYER0_ADDRESS  (LCD_FB_START_ADDRESS)

// Buffer state variable
static int32_t pending_buffer = -1;

// Function prototypes
static void SystemClock_Config(void);
static void OnError_Handler(uint32_t condition);
static void CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize);
static uint8_t LCD_Init(void);
void LTDC_Init(void);
static void LCD_BriefDisplay(void);

/**
 * @brief Error Handler.
 * @param condition Condition to check.
 *
 * This function turns on an LED and enters an infinite loop if the condition is true.
 */
static void OnError_Handler(uint32_t condition)
{
  if(condition)
  {
    BSP_LED_On(LED3);
    while(1) { ; } /* Blocking on error */
  }
}

/**
 * @brief Main function.
 *
 * Initializes the hardware, sets up the LCD, and enters an infinite loop.
 * @return int Program exit status.
 */
int main(void)
{
  uint8_t  lcd_status = LCD_OK;

  HAL_Init();
  SystemClock_Config();
  BSP_SDRAM_Init();

  lcd_status = LCD_Init();
  OnError_Handler(lcd_status != LCD_OK);

  BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
  BSP_LCD_SelectLayer(0);
  LCD_BriefDisplay();
  
  CopyBuffer((uint32_t *)image_320x240_argb8888, (uint32_t *)LAYER0_ADDRESS, 240, 160, 320, 240);
  pending_buffer = 0;
  
  HAL_DSI_Refresh(&hdsi_eval);

  while (1)
  {
      int ReadTemperature(void);
      HAL_Delay(2000);
  }
}

/**
 * @brief Simulates reading a temperature.
 *
 * @return int Simulated temperature value.
 */
int ReadTemperature(void){
    float minTemperature = 20.0;
    float maxTemperature = 30.0;
    float temperature = 28.0;
    return temperature;
}

/**
 * @brief Callback function for end of DSI refresh.
 * @param hdsi Pointer to DSI handler.
 *
 * Resets the pending_buffer variable when the DSI refresh ends.
 */
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
{
  if(pending_buffer >= 0)
  {
    pending_buffer = -1;
  }
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 6;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

static uint8_t LCD_Init(void)
{
  static DSI_PHY_TimerTypeDef PhyTimings;
  static DSI_CmdCfgTypeDef CmdCfg;
  static DSI_LPCmdTypeDef LPCmd;
  static DSI_PLLInitTypeDef dsiPllInit;
  static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  BSP_LCD_Reset();

  BSP_LCD_MspInit();

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 417;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  hdsi_eval.Instance = DSI;

  HAL_DSI_DeInit(&(hdsi_eval));

#if defined(USE_STM32469I_DISCO_REVA)
  dsiPllInit.PLLNDIV  = 100;
  dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV5;
#else
  dsiPllInit.PLLNDIV  = 125;
  dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV2;
#endif  /* USE_STM32469I_DISCO_REVA */
  dsiPllInit.PLLODF  = DSI_PLL_OUT_DIV1;

  hdsi_eval.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  hdsi_eval.Init.TXEscapeCkdiv = 0x4;
  HAL_DSI_Init(&(hdsi_eval), &(dsiPllInit));

  CmdCfg.VirtualChannelID      = 0;
  CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
  CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
  CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.ColorCoding           = DSI_RGB888;
  CmdCfg.CommandSize           = HACT;
  CmdCfg.TearingEffectSource   = DSI_TE_DSILINK;
  CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
  CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh      = DSI_AR_DISABLE;
  CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;
  HAL_DSI_ConfigAdaptedCommandMode(&hdsi_eval, &CmdCfg);

  LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
  LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
  LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
  LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
  LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
  LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
  LPCmd.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
  LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
  LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
  LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
  LPCmd.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;
  HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

  PhyTimings.ClockLaneHS2LPTime = 35;
  PhyTimings.ClockLaneLP2HSTime = 35;
  PhyTimings.DataLaneHS2LPTime = 35;
  PhyTimings.DataLaneLP2HSTime = 35;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 10;
  HAL_DSI_ConfigPhyTimer(&hdsi_eval, &PhyTimings);

  LTDC_Init();

  HAL_DSI_Start(&(hdsi_eval));

#if defined (USE_STM32469I_DISCO_REVC)
  NT35510_Init(NT35510_FORMAT_RGB888, LCD_ORIENTATION_LANDSCAPE);
#else
  OTM8009A_Init(OTM8009A_COLMOD_RGB888, LCD_ORIENTATION_LANDSCAPE);
#endif

  LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
  LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
  LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
  LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
  LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
  LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
  LPCmd.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
  LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
  LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
  LPCmd.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;
  HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

   HAL_DSI_ConfigFlowControl(&hdsi_eval, DSI_FLOW_CONTROL_BTA);
  HAL_DSI_Refresh(&hdsi_eval);

  return LCD_OK;
}

void LTDC_Init(void)
{
  hltdc_eval.Instance = LTDC;
  HAL_LTDC_DeInit(&hltdc_eval);

  hltdc_eval.Init.HorizontalSync = HSYNC;
  hltdc_eval.Init.VerticalSync = VSYNC;
  hltdc_eval.Init.AccumulatedHBP = HSYNC+HBP;
  hltdc_eval.Init.AccumulatedVBP = VSYNC+VBP;
  hltdc_eval.Init.AccumulatedActiveH = VSYNC+VBP+VACT;
  hltdc_eval.Init.AccumulatedActiveW = HSYNC+HBP+HACT;
  hltdc_eval.Init.TotalHeigh = VSYNC+VBP+VACT+VFP;
  hltdc_eval.Init.TotalWidth = HSYNC+HBP+HACT+HFP;

  hltdc_eval.Init.Backcolor.Blue = 0;
  hltdc_eval.Init.Backcolor.Green = 0;
  hltdc_eval.Init.Backcolor.Red = 0;

  hltdc_eval.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc_eval.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc_eval.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc_eval.Instance = LTDC;

  HAL_LTDC_Init(&hltdc_eval);
}

static void LCD_BriefDisplay(void)
{
  int tempRead = ReadTemperature();
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, 800, 112);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillRect(0, 112, 800, 368);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_DisplayStringAtLine(1, (uint8_t *)"      Room temperature");
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t *)"      This shows the temperature in the house ");
  BSP_LCD_DisplayStringAtLine(5, (uint8_t *)"      This is a proof of concept     ");
  char tempString[50];
  sprintf(tempString, "The temperature is: %d", tempRead);
  BSP_LCD_DisplayStringAtLine(10, (uint8_t *)tempString);
}

static void CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
  uint32_t destination = (uint32_t)pDst + (y * 800 + x) * 4;
  uint32_t source      = (uint32_t)pSrc;

  hdma2d.Init.Mode         = DMA2D_M2M;
  hdma2d.Init.ColorMode    = DMA2D_ARGB8888;
  hdma2d.Init.OutputOffset = 800 - xsize;

  hdma2d.XferCpltCallback  = NULL;

  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].InputOffset = 0;

  hdma2d.Instance          = DMA2D;

  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d, source, destination, xsize, ysize) == HAL_OK)
      {
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);
      }
    }
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
  while (1)
  {
  }
}
#endif
