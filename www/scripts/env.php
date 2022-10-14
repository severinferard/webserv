<html>
    <table>
        <?php
        $array = getenv();
        foreach ($array as $key => $item):  ?>
        <tr>
            <td><?php echo $key, '</td><td>', $item; ?></td>
        </tr>
        <?php endforeach; ?>
    </table>
</html>