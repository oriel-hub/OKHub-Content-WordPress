<?php
/**
 * The default template for displaying content
 *
 * Used for both single and index/archive/search.
 *
 * @package WordPress
 * @subpackage Twenty_Fifteen
 * @since Twenty Fifteen 1.0
 */
?>

<article id="post-<?php the_ID(); ?>" <?php post_class(); ?>>

	<?php
		// Post thumbnail.
		twentyfifteen_post_thumbnail();
	?>

	<header class="entry-header ">
		<?php
			if ( is_single() ) :
				the_title( '<h1 class="entry-title">', '</h1>' );
			else :
				the_title( sprintf( '<h2 class="entry-title"><a href="%s" rel="bookmark">', esc_url( get_permalink() ) ), '</a></h2>' );
			endif;
		?>
	</header><!-- .entry-header -->

	<div class="entry-content">
		<?php
			/* translators: %s: Name of current post */
			the_content( sprintf(
				__( 'Continue reading %s', 'twentyfifteen' ),
				the_title( '<span class="screen-reader-text">', '</span>', false )
			) );?>
  <div class="okhub-fields">
          <!-- Get the sources for which there is content in the post and, for each one, display the imported fields. -->
          <?php foreach (okhub_get_sources() as $source_code => $source_name) { ?>
            <?php $source = okhub_get_source($source_code); // Retrieve array with source information. See okhub.default.inc for a list of all available source's fields. ?>
          
            <h2>Content by <?php echo $source_name; ?> (<a href="#about_<?php echo $source_code; ?>">*</a>)</h2>
            
            <!------ Source fields ------>
            <!-- See the file "includes/okhub.settings.inc" for the complete list of available fields.
                 The field names used in Worpdress when importing content from the OKHub can be changed in that file. -->
            <!-- The function okhub_field is used to retrieve the fields values:
                  okhub_field($field_name, $source='', $language='', $before='', $after='', $format='', $separator=', '); -->
            <!-- <?php echo "$source_code - $source_name\n"; ?> -->
            <ul>
              <?php okhub_field(OKHUB_WP_FN_AUTHORS, $source_code, '', '<li class="okhub-field">' . __('Authors: '), '</li>', 'tags'); ?>
              <!-- If there is content for this field, the above is the same as:
                   <li class="okhub-field"> <?php __('Authors:'); ?> <?php okhub_field(OKHUB_WP_FN_AUTHORS, $source_code); ?> </li>
                   Except that passing the 'before' and 'after' strings to okhub_field() makes it unnecessary to check if
                   the field has some valid value. -->
              <?php okhub_field(OKHUB_WP_FN_PUBLISHER, $source_code, '', '<li class="okhub-field">' . __('Publisher: '), '</li>', 'tags'); ?>
              <?php okhub_field(OKHUB_WP_FN_LICENSE_TYPE, $source_code, '', '<li class="okhub-field">' . __('License type: '), '</li>'); ?>
              <!-- Translatable fields can be retrieved in a particular language. If left empty, the value in thepreferred language
                   will be returned (or in one of the alternative languages, if not available in the preferred language). 
                   In the case of non-translatable fields, if a language is passed it will be ignored. 
                   Please refer to the OKHub API documentation to know more about translatable fields. -->
              <?php okhub_field(OKHUB_WP_FN_LANGUAGE_NAME, $source_code, '', '<li class="okhub-field">' . __('Language: '), '</li>'); ?>
              <?php okhub_field(OKHUB_WP_FN_DATE_CREATED, $source_code, '', '<li class="okhub-field">' . __('Published on'), '</li>', array('date', 'Y/m/d')); ?>
              <?php okhub_field(OKHUB_WP_FN_DATE_UPDATED, $source_code, '', '<li class="okhub-field">' . __('Updated on: '), '</li>', array('date', 'Y/m/d')); ?>
              <!-- Alternatively: okhub_field(OKHUB_WP_FN_DATE_UPDATED, '', '<li class="okhub-field">' . __('Updated on: '), '</li>', 'date') would use the date format in Wordpress' settings.  -->
              <?php okhub_field(OKHUB_WP_FN_URLS, $source_code, '', '<li class="okhub-field">' . __('External URLs: '), '</li>', 'link'); ?>
              <!-- Alternatively: okhub_field(OKHUB_WP_FN_URLS, $source_code, '', '<li class="okhub-field">' . __('External URLs: '), '</li>', array('link', 'Some text')) can be used to display a text in the link instead of the URL -->
              <?php okhub_field(OKHUB_WP_FN_WEBSITE_URL, $source_code, '', '<li class="okhub-field">' . __('Source\'s URL: '), '</li>', 'link'); ?>
              <?php if (!empty($source[OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION])) { ?><li> License description: <?php echo $source[OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION]; ?></small> <?php } ?>
            </ul>

            <!------ Source categories ------>
            <!-- Call to okhub_post_categories_source() to check if there are categories from this source for this post.
                 The return value of this function could be used to display the categories,
                 without invoking to okhub_terms(), used here to show another way of displaying the categories. -->
            <?php if (okhub_post_categories_source($source_code)) { ?>
              <ul>
                <i><?php echo "$source_name " . __('Categories:'); ?></i>
                <!-- By default, the categories will be linked to their corresponding page. -->
                <?php okhub_terms('countries', $source_code, '<li class="okhub-field">' . __('Countries: '), '</li>'); ?>
                <?php okhub_terms('regions', $source_code, '<li class="okhub-field">' . __('Regions: '), '</li>'); ?>
                <?php okhub_terms('themes', $source_code, '<li class="okhub-field">' . __('Themes: '), '</li>'); ?>
                <?php okhub_terms('documenttypes', $source_code, '<li class="okhub-field">' . __('Document Types: '), '</li>'); ?>
              </ul>
            <?php
            } //if
            ?>

            <!------ Source description ------>
            <a name="about_<?php echo $source_code; ?>"></a>
            <p><small><?php echo (!empty($source[OKHUB_API_FN_SOURCE_DESCRIPTION])) ? $source[OKHUB_API_FN_SOURCE_DESCRIPTION] : ''; ?></small></p>
            <?php
            if (!empty($source[OKHUB_API_FN_SOURCE_WEBSITE])) {
              $source_website = $source[OKHUB_API_FN_SOURCE_WEBSITE];
              if (!empty($source[OKHUB_API_FN_SOURCE_LOGO])) {
                echo '<a href="'.$source_website.'"><img class="okhub-source-logo" src="'.$source[OKHUB_API_FN_SOURCE_LOGO].'"></a>';
              }
              else {
                echo '<small><a href="'.$source_website.'">'.$source_website.'</a></small>';
              }
            }
            ?>            
            
          <?php  
          } // foreach source
          ?>
        </ul>
      </div>
	  <?php 
			wp_link_pages( array(
				'before'      => '<div class="page-links"><span class="page-links-title">' . __( 'Pages:', 'twentyfifteen' ) . '</span>',
				'after'       => '</div>',
				'link_before' => '<span>',
				'link_after'  => '</span>',
				'pagelink'    => '<span class="screen-reader-text">' . __( 'Page', 'twentyfifteen' ) . ' </span>%',
				'separator'   => '<span class="screen-reader-text">, </span>',
			) );
		?>
	</div><!-- .entry-content -->

	<?php
		// Author bio.
		if ( is_single() && get_the_author_meta( 'description' ) ) :
			get_template_part( 'author-bio' );
		endif;
	?>

	<footer class="entry-footer">
		<?php twentyfifteen_entry_meta(); ?>
		<?php edit_post_link( __( 'Edit', 'twentyfifteen' ), '<span class="edit-link">', '</span>' ); ?>
	</footer><!-- .entry-footer -->

</article><!-- #post-## -->
