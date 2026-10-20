const { test, expect } = require('@playwright/test');

test('generated interaction and application run without handwritten JavaScript', async ({ page }) => {
  const errors = [];
  page.on('console', message => {
    if (message.type() === 'error') errors.push(message.text());
  });
  page.on('pageerror', error => errors.push(String(error)));

  await page.goto('/');
  await expect(page.getByRole('heading', { name: 'Slamhaus Examples' })).toBeVisible();
  await page.locator('#slam-left').fill('7');
  await page.locator('#slam-right').fill('5');
  await page.locator('#slam-calculate').click();
  await expect(page.locator('#slam-result')).toHaveText('12');

  const canvas = page.locator('#slam-game');
  await expect(canvas).toBeVisible();
  await expect.poll(async () => canvas.evaluate(element => element.width)).toBeGreaterThan(0);
  await expect.poll(async () => canvas.evaluate(element => element.height)).toBeGreaterThan(0);
  await expect(page.locator('link[rel="manifest"]')).toHaveAttribute(
    'href', '/manifest.webmanifest');
  expect(errors).toEqual([]);
});
